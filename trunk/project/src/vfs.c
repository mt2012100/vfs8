/*************************************************************************************************************
* File Name 		:	create_vfs.c
* Description		:	This file contains functions to create vfs, mount and unmount vfs
* Author		    :	Raghav Bali
* Date			    :	Sep 21 2012
* Version		    :	0.1
* Updates           :   [Raghav Bali Sep 21 2012]: Merged with the overall VFS code
**************************************************************************************************************/

#include "../include/main.h"

/*
    Create VFS in KBs
    returns 1,2,3,4 and 5 on failure to create,
    returns SUCCESS(1000) on success,
*/
int create_vfs(char *P1, int P2)
{
    /* dummy data to be written at the last block */
    char data_block='0';
    int i;
    long int block_size=1024;

    strcpy(vfs_header.label_name,P1);

    /* Check if similar named  */
    if(access( vfs_header.label_name, F_OK ) != -1 )
        return 1;
    else
    {
        /* Create VFS */
        if((vfs_file=fopen(vfs_header.label_name,"wb+"))==NULL)
        {
            /* could not create vfs */
            return 2;
        }
        else
        {
            /* cannot create VFS greate than 700MB */
            if(P2>734003200)
                return 3;
            /* size cannot be less than equal to 0 */
            else if(P2<=0)
                return 5;
            else
            {
                vfs_header.vfs_size=P2;
                /* Multiply block size to user defined size of VFS */
                block_size=(block_size*vfs_header.vfs_size)+sizeof(vfs_header);
                max_file_descriptors=vfs_header.vfs_size;//*1024;

                /* secure file size by writing data at the last byte of the file */
                fseek(vfs_file,block_size-1, SEEK_SET);
                fwrite(&data_block,sizeof(data_block),sizeof(data_block)*block_size,vfs_file);

                /* reset to beginning of file */
                rewind(vfs_file);
                /* initialize freelist */
                free_list=(char *)malloc(sizeof(char)*max_file_descriptors);

                for(i=0; i<max_file_descriptors/*MAXFILEDESCRIPTORS*/; i++)
                {
                    // vfs_header.free_list[i]='0';
                    free_list[i]='0';
                }

                file_descriptors=(file_descriptor_t *)malloc(sizeof(file_descriptor_t)*max_file_descriptors);

                /* write back the meta header */
                fwrite(&vfs_header,sizeof(vfs_header),1,vfs_file);

                /* newly added */
                fwrite(free_list,sizeof(free_list)*max_file_descriptors,1,vfs_file);
                fwrite(file_descriptors,sizeof(file_descriptors)*max_file_descriptors,1,vfs_file);
                /* modification ends */

                //free(free_list);
                //free(file_descriptors);
                fclose(vfs_file);

                /* success */
                return SUCCESS;
            }
        }
    }

}



/*
    Mount VFS
    returns 1,2,3 on failure to mount
    return SUCCESS(1000) on successful mount
*/
int mount_vfs(char vfs_name[])
{
    if(access( vfs_name, F_OK ) == -1 )
        return 1;
    else
    {

        if((vfs_file=fopen(vfs_name,"rb+"))==NULL)
        {
            /* failure to mount */
            return 2;
        }
        else
        {
            /* start from 1st byte of the file */
            strcpy(vfs_header.label_name,vfs_name);
            rewind(vfs_file);
            /* load the meta-header */
            fread(&vfs_header,sizeof(vfs_header),1,vfs_file);
            max_file_descriptors=vfs_header.vfs_size;/**1024;*/

            /* newly added code */
            free_list=(char *)malloc(sizeof(char)*max_file_descriptors);
            file_descriptors=(file_descriptor_t *)malloc(sizeof(file_descriptor_t)*max_file_descriptors);
            fread(free_list,sizeof(char)*max_file_descriptors,1,vfs_file);
            fread(file_descriptors,sizeof(file_descriptor_t)*max_file_descriptors,1,vfs_file);

            //free(free_list);
            //free(file_descriptors);
            /* modification eds */

            fclose(vfs_file);
            /* successfully loaded */
            return SUCCESS;
        }
    }
}



/*
    Unmount the VFS
    returns 0 on failure
    return 1 on success
*/
int unmount_vfs(char name[])
{
    /* err out if file does not exist */
    if(access( name, F_OK ) == -1 )
        return 1;
    /* err out if access is denied*/
    if(access( name, R_OK & W_OK ) == -1 )
        return 2;
    /* err out if cannot write */
    if((vfs_file=fopen(name,"rb+"))==NULL)
    {
        /* failure to write to file*/
        return 3;
    }
    else
    {
        rewind(vfs_file);
        fwrite(&vfs_header,sizeof(vfs_header),1,vfs_file);
        /* newly added */
        fwrite(free_list,sizeof(char)*max_file_descriptors,1,vfs_file);
        fwrite(file_descriptors,sizeof(file_descriptor_t)*max_file_descriptors,1,vfs_file);
        /* modification ends */
        // free(free_list);
        // free(file_descriptors);

        fclose(vfs_file);
        /* succefully written the data */
        return SUCCESS;
    }
}



