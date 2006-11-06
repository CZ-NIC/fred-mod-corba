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
 * need a reference can simply get it from notes. The reference is destroyed
 * automaticaly upon connection close.
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
 *         other modules.
 *   .
 * 
 */
