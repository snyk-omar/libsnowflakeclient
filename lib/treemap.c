/*
** Copyright (c) 2017-2018 Snowflake Computing, Inc. All rights reserved.
*/

#include <string.h>
#include "treemap.h"
#include "memory.h"


TREE_MAP * STDCALL sf_treemap_init()
{
    TREE_MAP *tree_map = (TREE_MAP *)SF_CALLOC(TREE_MAP_MAX_SIZE, sizeof(TREE_MAP));
    if (!tree_map)
    {
#ifdef SF_ERROR
        printf("[SF_ERROR] sf_treemap_init: Memory Allocation failed\n");
#endif
    }
    
    return tree_map;
}

/*
** The hash fxn used by the treemap
** should follow horner's rule 
** to generate index
** @return an index in the hashtable
*/
int STDCALL sf_treemap_hash_fxn(char *key)
{
    unsigned int iter = 0, len = 0;
    unsigned long hash = 0;

    if (!key)
    {
#ifdef SF_DEBUG
        printf("[SF_ERROR] sf_treemap_hash_fxn: Key passed for hashing is NULL\n");
#endif
        return -1;
    }

    len = strlen(key);

    for (iter = 0; iter < len; iter++)
    {
        hash = (HASH_CONSTANT*hash)+key[iter];
    }

#ifdef SF_INFO
    printf("[SF_INFO] sf_treemap_hash_fxn: hash calculated is %d",hash);
#endif
    return hash;
}

/* 
** Retrieves an index from the tree map
** to store a bind param at.
** @return - int
*/
unsigned long STDCALL sf_treemap_get_index(char *key)
{
    unsigned long hash = sf_treemap_hash_fxn(key);

    hash = hash % TREE_MAP_MAX_SIZE;
#ifdef SF_INFO
    printf("[SF_INFO] sf_treemap_get_index: index calculated is %d",hash);
#endif
    return hash;
}

/*
** Inserts the bind params at the right
** index in the tree map. This also takes
** care of collisions
** @return - void
*/

SF_RET_CODE STDCALL sf_treemap_insert_node(unsigned long index, TREE_MAP *tree_map, void *param, char *key)
{
    TREE_MAP *idx_cur;

    if (index < 0)
    {
#ifdef SF_DEBUG
        printf("[SF_ERROR] sf_treemap_insert_node: index is negative, no insertion will take place\n");
#endif
        return SF_RET_CODE_BAD_INDEX;
    }
    idx_cur = &tree_map[index];

    if (!idx_cur->tree)
    {
#ifdef SF_DEBUG
        printf("[SF_INFO] sf_treemap_insert_node: tree not been initialized yet\n");
#endif
        idx_cur->tree = rbtree_init();
    }

    return rbtree_insert(&idx_cur->tree, param, key);
}

/* sf_treemap_extract_node
** extract a node corresponding to a key
** from the treemap
** @return - void
*/

SF_RET_CODE STDCALL sf_treemap_extract_node(TREE_MAP *tree_map, int idx, char *key, void **ret_param)
{
    TREE_MAP *cur_node;
    SF_RET_CODE retval = SF_RET_CODE_ERROR;
    if (tree_map && (cur_node = &tree_map[idx]))
    {
        *ret_param = rbtree_search_node(cur_node->tree, key);
#ifdef SF_DEBUG
        if (!ret_param)
        {
            printf("sf_treemap_extract_node: Node not found");
            retval = SF_RET_CODE_NODE_NOT_FOUND;
            goto done;
        }
#endif
    }
#ifdef SF_DEBUG
    else
    {
        if (!tree_map)
        {
            printf("[SF_ERROR] sf_treemap_extract_node: tree_map passed is NULL");
            goto done;
        }
        else if (!cur_node)
        {
            printf("[SF_ERROR] sf_treemap_extract_node: cur_node is NULL\n");
            goto done;
        }
    }
#endif /*SF_DEBUG*/
    retval = SF_RET_CODE_SUCCESS;
done:

    return retval;
}

/* sf_treemap_set
** insert SF_BIND_INPUT into treemap
** @return- void
*/

SF_RET_CODE STDCALL sf_treemap_set(TREE_MAP *tree_map, void *param, char *key)
{

    if (!param)
    {
        /* Handle error 
        ** return or goto done;
        */
        #ifdef SF_DEBUG
        printf("sf_treemap_set: Param passed is NULL\n");
        #endif
        return SF_RET_CODE_ERROR;
    }

    return sf_treemap_insert_node(sf_treemap_get_index(key), tree_map, param, key);
}

/*
** sf_treemap_get
** get params corresponding to a key from the treemap
** @return void *
*/
void * STDCALL sf_treemap_get(TREE_MAP *tree_map, char *key)
{
    void *param = NULL;
    if (!tree_map || !key)
    {
        return NULL;
    }

    sf_treemap_extract_node(tree_map, sf_treemap_get_index(key), key, &param);
#ifdef SF_DEBUG
    if (!param)
    {
        printf("[SF_INFO] sf_treemap_get: param returned is NULL\n");
    }
#endif
    return param;
}

/* sf_treemap_deallocate
** deallocate treemap and any assoicated RedBlackTrees.
** @return SF_STATUS
*/
void STDCALL sf_treemap_deallocate(TREE_MAP *tree_map)
{
    int iter = 0;

    if (!tree_map)
    {
#ifdef SF_DEBUG
        printf("[SF_ERROR] sf_treemap_deallocate: treemap is NULL\n");
#endif
        return;
    }
    for (iter = 0; iter < TREE_MAP_MAX_SIZE; iter++)
    {
        if (tree_map[iter].tree)
        {
            rbtree_deallocate(tree_map[iter].tree);
        }
        tree_map[iter].tree = NULL;
    }
    SF_FREE(tree_map);
}
