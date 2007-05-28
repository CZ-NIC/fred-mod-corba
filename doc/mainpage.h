/**
 * @file mainpage.h
 *
 * This file contains only the main page of doxygen documentation.
 */

/**
 * @mainpage package mod_corba
 *
 * @section overview Overview
 *
 * This module is a manager of corba references. It obtains references
 * from corba nameservice when it is enabled for connection. Those references
 * are exported under aliases in connection notes table. Other modules which
 * need a reference can simply get it from notes table. The reference is
 * destroyed automaticaly upon connection close.
 *
 * @section config Module's configuration
 *
 * List of configuration directives recognized by mod_corba:
 * 
 *   name: CorbaEnable
 *   - value:        On, Off
 *   - default:      Off
 *   - context:      global config, virtual host
 *   - description:
 *         Activates corba module.
 *   .
 * 
 *   name: CorbaNameservice
 *   - value:        host[:port]
 *   - default:      localhost
 *   - context:      global config, virtual host
 *   - description:
 *         A location of CORBA nameservice where the module asks for objects.
 *   .
 * 
 *   name: CorbaObject
 *   - value:        object alias
 *   - default:      none
 *   - context:      global config, virtual host
 *   - description:
 *         Object is name from nameservice and will be exported under alias for
 *         other modules. Format of object is CONTEXTNAME.OBJECTNAME. If context
 *         part is missing then default context 'fred' is assumed.
 *   .
 * 
 * CorbaNameservice and CorbaObject configuration values are in virtual servers
 * inherited from main server, which can be exploited to set these settings
 * just once for all servers. CorbaEnable must be enabled explicitly for each
 * virtual server - this directive is not inherited.
 *
 * mod_corba alone is not meaningfull. It is intended to be used by other
 * modules. For reasonable example of mod_corba's configuration in conjunction
 * with other modules see mod_eppd's or mod_whoisd's documentation.
 *
 * @section make Building and installing the module
 *
 * Module comes with configure script, which should hide differences
 * among Fedora, Gentoo, Debian and Ubuntu linux distributions. Other
 * distribution let alone UNIX-like operating systems where not tested.
 * The following parameters in addition to standard ones are recognized
 * by the configure script and don't have to be ussualy specified since
 * tools' location is automatically found by configure in most cases:
 *
 *     - --with-apr-config      Location of apr-config tool.
 *     - --with-apxs            Location of apxs tool.
 *     - --with-pkg-config      Location of pkg-config tool.
 *     - --with-doc             Location of doxygen if you want to generate documentation.
 *     .
 *
 * The installation directories are not taken into account. The installation
 * directories are given by apxs tool.
 *
 * The module is built by the traditional way: ./configure && make && make
 * install. The module is installed in directory where reside other apache
 * modules.
 *
 * @section trouble Troubleshooting
 *
 * There is not any test utility which would make debugging easier as in
 * case of mod_eppd or mod_whoisd modules.
 */
