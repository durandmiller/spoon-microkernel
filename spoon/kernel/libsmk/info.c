#include <smk.h>


/**  \defgroup INFO  Information and Accounting  
 *
 */

/** @{ */



int 	smk_process_get_list( int max, struct process_information *list )
{
      return _sysenter( (SYS_INFO|SYS_ONE), 
	  					(unsigned int)max, 
						(unsigned int)list, 0, 0, 0); 
}



/** @} */

