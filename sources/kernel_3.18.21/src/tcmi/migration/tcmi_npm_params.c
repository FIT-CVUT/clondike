/**
 * @file tcmi_npm_params.c - non-preemptive parameters utils.
 *                      
 */

#include "tcmi_npm_params.h"
#include <asm/uaccess.h>
#include <linux/module.h>

#include <dbg.h>

/*
 * @copied_from_kernel
 *
 * count() counts the number of strings in array user char array
 */
static int count(const char __user * const __user * argv, int max)
{
	int i = 0;

	if (argv != NULL) {
		for (;;) {
			const char __user * p;

			if (get_user(p, argv))
				return -EFAULT;
			if (!p)
				break;
			argv++;
			if(++i > max)
				return -E2BIG;
			cond_resched();
		}
	}
	return i;
}

static int copy_strings(int argc, const char __user * const __user * argv,
			struct tcmi_npm_params* params, int* remaining_max_length)
{
	int ret = 0, i;

	(*remaining_max_length) -= reserve_array_length(argc); // Reserve space for the array pointing to strings

	for (i = 0; i < argc; i++ ) {
		const char __user *str;
		int len, err;

		if (get_user(str, (argv+i)) ||
				!(len = strnlen_user(str, *remaining_max_length))) {
			ret = -EFAULT;
			goto out;
		} // Retrieved len includes trailing zero
		
		if ( *remaining_max_length < len)  {
			ret = -E2BIG;
			goto out;
		}

		err = copy_from_user(params->data + NPM_DATA_SIZE_MAX - (*remaining_max_length), str, len);
		if (err) {
			ret = -EFAULT;
			goto out;
		}

		*remaining_max_length -= len;		
	}	
out:
	return ret;
}

int extract_tcmi_npm_params(struct tcmi_npm_params* params, const char * filename, 
			const char __user * const __user *argv, const char __user * const __user *envp) {
	int remaining_max = NPM_DATA_SIZE_MAX;
	int err;

	if ( !params )
		return -EINVAL;

	if ( strlen(filename)+1 > NPM_FILENAME_MAX )
		return -EINVAL;

	strcpy(params->file_name, filename);

	params->argsc = count(argv, NPM_DATA_SIZE_MAX/sizeof(void*));
	params->envpc = count(envp, NPM_DATA_SIZE_MAX/sizeof(void*));

	if ( (err = copy_strings(params->argsc, argv, params, &remaining_max)) )
		return err;
	if ( (err = copy_strings(params->envpc, envp, params, &remaining_max)) )
		return err;

	params->used_data_length = NPM_DATA_SIZE_MAX - remaining_max;

	mdbg(INFO4, "Filename: %s, Argc: %d, Evnpc: %d Used data: %d", params->file_name, params->argsc, params->envpc, params->used_data_length);

  return fixup_npm_params_pointers(params);


};

/**
 * Given a npm params structure, this method will fixup args and envp pointers to point to correct places 
 * in the data member.
 */
int fixup_npm_params_pointers(struct tcmi_npm_params* params) {
    int i, pos =0;

    // Decode args array pointer
    params->args = (char**)params->data;
    pos += reserve_array_length(params->argsc);
    // Fill in pointers in arg arrays pointer
    for ( i=0; i < params->argsc; i++ ) {
        int len = strlen(params->data+pos);

        params->args[i] = params->data + pos;
        pos += (len+1);
    }
    params->args[i] = NULL;

    // Decode envp array pointer
    params->envp = (char**)(params->data + pos);
    pos += reserve_array_length(params->envpc);
    // Fill in pointers in envp arrays pointer
    for ( i=0; i < params->envpc; i++ ) {
        int len = strlen(params->data+pos);

        params->envp[i] = params->data + pos;
        pos += (len+1);
    }
    params->envp[i] = NULL;

    return 0;
};


