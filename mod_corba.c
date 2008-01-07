/*  
 *  Copyright (C) 2007  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */
 
/**
 * @file mod_corba.c
 *
 * mod_corba.c manages corba object references.
 *
 * The management is at this time rather primitive. For each connection, 
 * for which this module was enabled, are created configured object
 * references, which are saved in connection notes and available for later
 * use by other modules. The references are automaticaly cleaned up upon
 * connection pool destruction.
 */

#include "httpd.h"
#include "http_log.h"
#include "http_config.h"
#include "http_connection.h"	/* connection hooks */

#include "apr_pools.h"
#include "apr_strings.h"
#include "apr_hash.h"
#include "apr_tables.h"

#include <orbit/orbit.h>
#include <ORBitservices/CosNaming.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/**
 * corba_module declaration.
 */
module AP_MODULE_DECLARE_DATA corba_module;

/**
 * Configuration structure of corba module.
 */
typedef struct {
	int          enabled;     /**< Whether mod_corba is enabled for host. */
	const char  *ns_loc;      /**< Location of CORBA nameservice. */
	apr_table_t *objects;     /**< Names and aliases of managed objects. */
    apr_table_t *objects_ior; /**< Cached IORs for managed objects (key = object alias). */
	CORBA_ORB    orb;         /**< Variables needed for corba submodule. */
}corba_conf;

#if AP_SERVER_MINORVERSION_NUMBER == 0
/**
 * ap_log_cerror is defined only if apache version is 2.0 because 2.0
 * contrary to 2.2 does not have this function.
 */
#define ap_log_cerror(mark, level, status, c, ...) \
	ap_log_error(mark, level, status, (c)->base_server, __VA_ARGS__)
#endif

/** Quick test if corba exception was raised. */
#define raised_exception(ev)    ((ev)->_major != CORBA_NO_EXCEPTION)

/**
 * This structure is passed to reference cleanup routine.
 *
 * We pass also the connection id in order to track references when
 * debug log is enabled.
 */
struct reference_cleanup_arg {
	void            *service;
	const conn_rec  *c;
	const char      *alias;
};

/**
 * Cleanup routine releases corba object reference.
 *
 * This routine is called upon destroying connection pool.
 *
 * @param object  The object reference.
 */
static apr_status_t reference_cleanup(void *raw_arg)
{
	CORBA_Environment ev[1];
	struct reference_cleanup_arg *arg = raw_arg;

	CORBA_exception_init(ev);

	/* releasing managed object */
	CORBA_Object_release(arg->service, ev);
	if (raised_exception(ev)) {
		ap_log_cerror(APLOG_MARK, APLOG_ERR, 0, arg->c,
			"mod_corba: error when releasing corba object: %s.",
			ev->_id);
		return APR_EGENERAL;
	}
	CORBA_exception_free(ev);

	ap_log_cerror(APLOG_MARK, APLOG_DEBUG, 0, arg->c,
		"mod_corba: reference with alias '%s', belonging to "
		"connection %ld was released.", arg->alias, arg->c->id);
	return APR_SUCCESS;
}

/** 
 * Context structure passed between get_reference_from_() and connection
 * handler. 
 */
struct get_reference_ctx {
	conn_rec	*c;         /**< Current connection. */
	CORBA_ORB   orb;        /**< Orb. */
	apr_hash_t	*objects;   /**< Hash table of object references. */
};

/** 
 * Context structure passed between get_ior_from_nameservice() and post config
 * hook. 
 */
struct get_ior_ctx {
    server_rec  *s;                     /**< Current server. */
    CORBA_ORB   orb;                    /**< Orb. */
    CosNaming_NamingContext nameservice;/**< Corba nameservice. */
    apr_table_t *objects_ior;           /**< Table for storing pair alias-IOR.*/
    apr_pool_t *pool;                   /**< Temporary pool pointer. */
};

/**
 * Function obtains IOR string for one object registered in mod_corba and
 * assign it to configuration table
 *
 * @param pctx    Context pointer.
 * @param name    Name of object.
 * @param alias   Alias of object.
 * @return        1 if successfull, 0 in case of failure.
 */
static int get_ior_from_nameservice(void *pctx, const char *name, const char *alias)
{
	void	    *service;
	const char	*p;
    char        *ior;
	CORBA_Environment	    ev[1];
    CosNaming_Name	        cos_name;
    CosNaming_NameComponent	name_component[2] = { {NULL, "context"},
		{NULL, "Object"} };
	
    struct reference_cleanup_arg *cleanup_arg;
    struct get_ior_ctx *ctx = pctx;

	/* divide name in two parts - context and object's name */
	for (p = name; *p != '\0'; p++) {
		if (*p == '.') break;
	}
	if (*p == '\0') {
		name_component[0].id = apr_pstrdup(ctx->pool, "fred");
		name_component[1].id = (char *) name;
	}
	else {
		name_component[0].id = apr_pstrmemdup(ctx->pool, name,
				p - name);
		name_component[1].id = apr_pstrdup(ctx->pool, p + 1);
	}
	cos_name._maximum = cos_name._length = 2;
	cos_name._buffer = name_component;
	
    /* get object's reference */
	CORBA_exception_init(ev);
	service = CosNaming_NamingContext_resolve(ctx->nameservice, &cos_name,
			ev);
	if (service == CORBA_OBJECT_NIL || raised_exception(ev)) {
		ap_log_error(APLOG_MARK, APLOG_ERR, 0, ctx->s,
			"mod_corba: Could not obtain reference of "
			"object '%s': %s.", name,
			(ev->_id) ? ev->_id : "Unknown error");
		CORBA_exception_free(ev);
		return 0;
	}

    /* translate it to IOR string */
    ior = CORBA_ORB_object_to_string(ctx->orb, service, ev);
    if (raised_exception(ev)) {
		ap_log_error(APLOG_MARK, APLOG_ERR, 0, ctx->s,
			"mod_corba: Could not obtain IOR string from "
			"object '%s': %s.", name,
			(ev->_id) ? ev->_id : "Unknown error");
		CORBA_exception_free(ev);
		return 0;
    }
    	
    /* save IOR in connection notes */
	apr_table_set(ctx->objects_ior, alias, ior);

    ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, ctx->s,
            "mod_corba: (%s) Stored object '%s' IOR string: '%s'", 
            ctx->s->defn_name, name, ior);

    CORBA_Object_release(service, ev);

	return 1;

}
/**
 * Function obtains one reference from IOR string and sticks the
 * reference to connection.
 *
 * @param pctx    Context pointer.
 * @param name    Name of object.
 * @param alias   Alias of object.
 * @return        1 if successfull, 0 in case of failure.
 */
static int get_reference_from_ior(void *pctx, const char *alias, const char *ior)
{
    void *service;
    CORBA_Environment ev[1];
    struct reference_cleanup_arg    *cleanup_arg;
    struct get_reference_ctx *ctx = pctx;
    
    CORBA_exception_init(ev);
    service = CORBA_ORB_string_to_object(ctx->orb, ior, ev);
  
	if (raised_exception(ev)) {
		ap_log_cerror(APLOG_MARK, APLOG_ERR, 0, ctx->c,
			"mod_corba: Could not obtain reference of "
			"object alias '%s' from IOR '%s': %s.", alias, ior,
			(ev->_id) ? ev->_id : "Unknown error");
		CORBA_exception_free(ev);
		return 0;
	}
    /* TODO: fallback try to get reference using nameservice */

	/* register cleanup routine for reference */
	cleanup_arg = apr_palloc(ctx->c->pool, sizeof *cleanup_arg);
	cleanup_arg->c       = ctx->c;
	cleanup_arg->alias   = alias;
	cleanup_arg->service = service;
	apr_pool_cleanup_register(ctx->c->pool, cleanup_arg, reference_cleanup,
			apr_pool_cleanup_null);

	/* save object in connection notes */
	apr_hash_set(ctx->objects, alias, strlen(alias), service);

	ap_log_cerror(APLOG_MARK, APLOG_DEBUG, 0, ctx->c,
		"mod_corba: reference for alias '%s', belonging to "
		"connection %ld was obtained from IOR.", alias, ctx->c->id);

	return 1;
}
/**
 * Fallback function for obtain one reference for object from nameservice
 */
static int get_reference_from_nameservice(void *pctx, const char *alias);

/**
 * Connection handler.
 *
 * Connection handler obtains object references from IOR string for
 * configured objects. These object references are sticked to connection
 * for later use by other modules. Cleanup routine which handles
 * reference's release is bound to connection.
 *
 * @param c   Incoming connection.
 * @return    Return code
 */
static int corba_process_connection(conn_rec *c)
{
	struct get_reference_ctx	ctx;
	server_rec  *s = c->base_server;
	corba_conf  *sc = (corba_conf *)
		ap_get_module_config(s->module_config, &corba_module);

	/* do nothing if corba is disabled */
	if (!sc->enabled)
		return DECLINED;

	/* init ctx structure and obtain references for all configured objects */
	ctx.c = c;
    ctx.orb = sc->orb;
	ctx.objects = apr_hash_make(c->pool);
	apr_table_do(get_reference_from_ior, (void *) &ctx, sc->objects_ior, NULL);

	/* bind hash table of object references to conn_rec */
	ap_set_module_config(c->conn_config, &corba_module, ctx.objects);

	return DECLINED;
}

/**
 * Cleanup routine releases ORB.
 *
 * This routine is called upon destroying configuration pool (which is at
 * restarts).
 *
 * @param par_orb  The ORB.
 */
static apr_status_t corba_cleanup(void *par_orb)
{
	CORBA_Environment ev[1];
	CORBA_ORB	orb = (CORBA_ORB) par_orb;

	CORBA_exception_init(ev);

	/* tear down the ORB */
	CORBA_ORB_destroy(orb, ev);
	if (raised_exception(ev)) {
		ap_log_error(APLOG_MARK, APLOG_ERR, 0, NULL,
			"mod_corba: error when releasing ORB: %s.", ev->_id);
		CORBA_exception_free(ev);
		return APR_EGENERAL;
	}
	ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, NULL,
			"mod_corba: global ORB released");
	return APR_SUCCESS;
}

/**
 * In post config hook we initialize ORB
 *
 * @param p     Memory pool.
 * @param plog  Memory pool used for logging.
 * @param ptemp Memory pool destroyed right after postconfig phase.
 * @param s     Server record.
 * @return      Status.
 */
static int corba_postconfig_hook(apr_pool_t *p, apr_pool_t *plog,
		apr_pool_t *ptemp, server_rec *s)
{
	corba_conf	*sc;
	CORBA_ORB	orb;
	CORBA_Environment	ev[1];
    CosNaming_NamingContext nameservice;
   
    char	ns_string[150];
    struct get_ior_ctx  ctx;

    /*
	 * do initialization of corba
	 */
	CORBA_exception_init(ev);
	/* create orb object */
	orb = CORBA_ORB_init(0, NULL, "orbit-local-orb", ev);
	if (raised_exception(ev)) {
		ap_log_error(APLOG_MARK, APLOG_CRIT, 0, s,
			"mod_corba: could not create ORB: %s.", ev->_id);
		CORBA_exception_free(ev);
		return HTTP_INTERNAL_SERVER_ERROR;
	}
	/* register cleanup for ORB */
	apr_pool_cleanup_register(p, orb, corba_cleanup,
			apr_pool_cleanup_null);
	ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, NULL,
			"mod_corba: global ORB initialized");

	/*
	 * Iterate through available servers and if corba is enabled
	 * initialize ORB reference for that server.
	 */
	while (s != NULL) {
		sc = (corba_conf *) ap_get_module_config(s->module_config,
				&corba_module);

		if (sc->enabled) {
			/* set default values for object lookup data */
			if (sc->ns_loc == NULL)
				sc->ns_loc = apr_pstrdup(p, "localhost");
			if (apr_is_empty_table(sc->objects))
				ap_log_error(APLOG_MARK, APLOG_WARNING, 0, s,
					"mod_corba: module enabled but no "
					"objects to manage were configured!");
			sc->orb = orb;

            /* get nameservice's reference */
            ns_string[149] = 0;
            snprintf(ns_string, 149, "corbaloc::%s/NameService", sc->ns_loc);
            nameservice = (CosNaming_NamingContext)
   	        CORBA_ORB_string_to_object(sc->orb, ns_string, ev);
            if (nameservice == CORBA_OBJECT_NIL || raised_exception(ev))
	        {
	            ap_log_error(APLOG_MARK, APLOG_ERR, 0, s,
	            "mod_corba: could not obtain reference to "
	            "CORBA nameservice: %s.",
	            (ev->_id) ? ev->_id : "Unknown error");
	            CORBA_exception_free(ev);
	            return HTTP_INTERNAL_SERVER_ERROR;	
            }

            /* get IOR strings for all registred objects */
            ctx.s = s;
            ctx.orb = orb;
            ctx.nameservice = nameservice;
            ctx.objects_ior = sc->objects_ior;
            ctx.pool = ptemp;
            apr_table_do(get_ior_from_nameservice, (void *) &ctx, sc->objects, NULL);

            /* release nameservice */
            CORBA_Object_release(nameservice, ev);
            if (raised_exception(ev))
            {
	            ap_log_error(APLOG_MARK, APLOG_ERR, 0, s,
                "mod_corba: error when releasing nameservice's "
	        	"reference: %s.", ev->_id);
                CORBA_exception_free(ev);
            }

        }
		s = s->next;
	}
    
	ap_log_error(APLOG_MARK, APLOG_INFO, 0, s, "mod_corba: initialized "
			"successfully");

	return OK;
}

/**
 * Handler for apache's configuration directive "CorbaEnable".
 *
 * @param cmd    Command structure.
 * @param dummy  Not used parameter.
 * @param flag   1 means Corba is turned on, 0 means turned off.
 * @return       Error string in case of failure otherwise NULL.
 */
static const char *set_corba(cmd_parms *cmd, void *dummy, int flag)
{
	server_rec *s = cmd->server;
	corba_conf *sc = (corba_conf *)
		ap_get_module_config(s->module_config, &corba_module);

	const char *err = ap_check_cmd_context(cmd,
			NOT_IN_DIR_LOC_FILE | NOT_IN_LIMIT);
	if (err)
		return err;

	sc->enabled = flag;
	return NULL;
}

/**
 * Handler for apache's configuration directive "CorbaNameservice".
 * Sets the host and optional port where nameservice runs.
 *
 * @param cmd    Command structure.
 * @param dummy  Not used parameter.
 * @param ns_loc The host [port] of nameservice.
 * @return       Error string in case of failure otherwise NULL.
 */
static const char *set_nameservice(cmd_parms *cmd, void *dummy,
		const char *ns_loc)
{
	const char *err;
	server_rec *s = cmd->server;
	corba_conf *sc = (corba_conf *)
		ap_get_module_config(s->module_config, &corba_module);

	err = ap_check_cmd_context(cmd, NOT_IN_DIR_LOC_FILE|NOT_IN_LIMIT);
	if (err)
		return err;

	/*
	 * catch double definition of location
	 * that's not serious fault so we will just print message in log
	 */
	if (sc->ns_loc != NULL) {
		ap_log_error(APLOG_MARK, APLOG_ERR, 0, s,
			"mod_corba: more than one definition of nameserice "
			"location. All but the first one will be ignored");
		return NULL;
	}

	sc->ns_loc = ns_loc;

	return NULL;
}

/**
 * Handler for apache's configuration directive "CorbaObject".
 * Sets a name of CORBA object which will be managed by this module.
 *
 * @param cmd      Command structure.
 * @param dummy    Not used parameter.
 * @param object   A name of object.
 * @return         Error string in case of failure otherwise NULL.
 */
static const char *set_object(cmd_parms *cmd, void *dummy, const char *object,
		const char *alias)
{
	const char  *err;
	server_rec  *s = cmd->server;
	corba_conf  *sc = (corba_conf *)
		ap_get_module_config(s->module_config, &corba_module);

	err = ap_check_cmd_context(cmd, NOT_IN_DIR_LOC_FILE|NOT_IN_LIMIT);
	if (err)
		return err;

	apr_table_set(sc->objects, object, alias);

	return NULL;
}

/**
 * Structure containing mod_corba's configuration directives and their
 * handler references.
 */
static const command_rec corba_cmds[] = {
	AP_INIT_FLAG("CorbaEnable", set_corba, NULL, RSRC_CONF,
		 "Whether corba object manager is enabled or not"),
	AP_INIT_TAKE1("CorbaNameservice", set_nameservice, NULL, RSRC_CONF,
		 "Location of CORBA nameservice (host[:port]). Default is "
		 "localhost."),
	AP_INIT_TAKE2("CorbaObject", set_object, NULL, RSRC_CONF,
		 "Context and name of object to provision and its alias. "
		 "Format for context and name is CONTEXTNAME.OBJECTNAME."),
	{ NULL }
};

/**
 * Initialization of of mod_corba's configuration structure.
 */
static void *create_corba_config(apr_pool_t *p, server_rec *s)
{
	corba_conf *sc = (corba_conf *) apr_pcalloc(p, sizeof(*sc));

	sc->enabled = 0;
	sc->ns_loc = NULL;
	sc->orb = NULL;
	sc->objects = apr_table_make(p, 5);
    sc->objects_ior = apr_table_make(p, 5);

	return sc;
}

/**
 * Merge of of mod_corba's configuration structure.
 */
static void *merge_corba_config(apr_pool_t *p, void *base_par,
		void *override_par)
{
	corba_conf *base = (corba_conf *) base_par;
	corba_conf *override = (corba_conf *) override_par;

	/* we will allow to inherit only ns_loc and objects */
	apr_table_overlap(base->objects, override->objects,
			APR_OVERLAP_TABLES_SET);
	if (override->ns_loc == NULL)
		override->ns_loc = base->ns_loc;
	/* copy of orb is useless since it is NULL for sure */

	return override;
}

/**
 * Registration of various hooks which the mod_corba is interested in.
 */
static void register_hooks(apr_pool_t *p)
{
	ap_hook_post_config(corba_postconfig_hook, NULL, NULL, APR_HOOK_MIDDLE);
	ap_hook_process_connection(corba_process_connection, NULL, NULL,
			APR_HOOK_MIDDLE);
}

/**
 * corba_module definition.
 */
module AP_MODULE_DECLARE_DATA corba_module = {
    STANDARD20_MODULE_STUFF,
    NULL,                       /* create per-directory config structure */
    NULL,                       /* merge per-directory config structures */
    create_corba_config,        /* create per-server config structure */
    merge_corba_config,         /* merge per-server config structures */
    corba_cmds,                 /* command apr_table_t */
    register_hooks              /* register hooks */
};

/* vi:set ts=8 sw=8: */
