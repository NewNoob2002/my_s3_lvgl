#include "lv_port.h"
#include "SdFat.h"
#include "HAL.h"

/*********************
 *      DEFINES
 *********************/
#define SD_LETTER '/'
#define SD_USE_SEM 0
/**********************
 *      TYPEDEFS
 **********************/

/* Create a type to store the required data about your file.*/
typedef FsFile file_t;

/*Similarly to `file_t` create a type for directory reading too */
typedef FsFile rddir_t;

#define SD_FILE(file_p) ((file_t*)file_p)
#define SD_DIR(dir_p)   ((rddir_t*)dir_p)

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void fs_init(void);

static bool fs_ready(lv_fs_drv_t * drv);
static void * fs_open(lv_fs_drv_t * drv, const char * path, lv_fs_mode_t mode);
static lv_fs_res_t fs_close(lv_fs_drv_t * drv, void * file_p);
static lv_fs_res_t fs_read(lv_fs_drv_t * drv, void * file_p, void * buf, uint32_t btr, uint32_t * br);
static lv_fs_res_t fs_write(lv_fs_drv_t * drv, void * file_p, const void * buf, uint32_t btw, uint32_t * bw);
static lv_fs_res_t fs_seek(lv_fs_drv_t * drv, void * file_p, uint32_t pos, lv_fs_whence_t whence);
// static lv_fs_res_t fs_size(lv_fs_drv_t * drv, void * file_p, uint32_t * size_p);
static lv_fs_res_t fs_tell(lv_fs_drv_t * drv, void * file_p, uint32_t * pos_p);

static void * fs_dir_open(lv_fs_drv_t * drv, const char * path);
static lv_fs_res_t fs_dir_read(lv_fs_drv_t * drv, void * rddir_p, char * fn);
static lv_fs_res_t fs_dir_close(lv_fs_drv_t * drv, void * rddir_p);

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void lv_port_fs_init()
{
    fs_init();
        /*---------------------------------------------------
     * Register the file system interface in LVGL
     *--------------------------------------------------*/

    /*Add a simple drive to open images*/
    static lv_fs_drv_t fs_drv;
    lv_fs_drv_init(&fs_drv);

    /*Set up fields...*/
    fs_drv.letter = SD_LETTER;
    fs_drv.cache_size = 1024;                     /*0: Default cache size. >0: Size of the cache (in bytes) to be used by the system*/
    fs_drv.ready_cb = fs_ready;
    fs_drv.open_cb = fs_open;
    fs_drv.close_cb = fs_close;
    fs_drv.read_cb = fs_read;
    fs_drv.write_cb = fs_write;
    fs_drv.seek_cb = fs_seek;
    fs_drv.tell_cb = fs_tell;

    fs_drv.dir_close_cb = fs_dir_close;
    fs_drv.dir_open_cb = fs_dir_open;
    fs_drv.dir_read_cb = fs_dir_read;

    lv_fs_drv_register(&fs_drv);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
/* Initialize your Storage device and File system. */
static void fs_init(void)
{
}


static bool fs_ready(lv_fs_drv_t * drv)
{
    return online.microSd;
}


/**
 * Open a file
 * @param drv pointer to a driver where this function belongs
 * @param path path to the file beginning with the driver letter (e.g. /folder/file.txt)
 * @param mode read: FS_MODE_RD, write: FS_MODE_WR, both: FS_MODE_RD | FS_MODE_WR
 * @return pointer to a file_t variable
 */
static void * fs_open (lv_fs_drv_t * drv, const char * path, lv_fs_mode_t mode)
{
#if SD_USE_SEM
    if (xSemaphoreTake(sdCardSemaphore, 10) != pdPASS)
    {
        LV_LOG_ERROR("sdCardSemaphore take failed in fs_open");
        return NULL;
    }
    markSemaphore(FUNCTION_LVGL_OPEN);
#endif
    oflag_t oflag = O_RDONLY;

    if(mode == LV_FS_MODE_WR)
    {
        oflag = O_WRONLY;
    }
    else if(mode == LV_FS_MODE_RD)
    {
        oflag = O_RDONLY;
    }
    else if(mode == (LV_FS_MODE_WR | LV_FS_MODE_RD))
    {
        oflag = O_RDWR | O_CREAT;
    }

    file_t* file_p = new file_t;

    if(file_p == NULL)
    {
        LV_LOG_ERROR("file_p is NULL");
#if SD_USE_SEM
        xSemaphoreGive(sdCardSemaphore);
#endif
        return NULL;
    }

    if(!file_p->open(path, oflag))
    {
        LV_LOG_ERROR("file_p->open failed");
        delete file_p;
        file_p = NULL;
    }
#if SD_USE_SEM
    xSemaphoreGive(sdCardSemaphore);
#endif
    return file_p;
}

/**
 * Close an opened file
 * @param drv pointer to a driver where this function belongs
 * @param file_p pointer to a file_t variable. (opened with lv_ufs_open)
 * @return LV_FS_RES_OK: no error, the file is read
 *         any error from lv_fs_res_t enum
 */
static lv_fs_res_t fs_close (lv_fs_drv_t * drv, void * file_p)
{
#if SD_USE_SEM
    if (xSemaphoreTake(sdCardSemaphore, 10) != pdPASS)
    {
        LV_LOG_ERROR("sdCardSemaphore take failed in fs_close");
        return NULL;
    }
    markSemaphore(FUNCTION_LVGL_CLOSE);
#endif
    lv_fs_res_t res = LV_FS_RES_NOT_IMP;
    res = SD_FILE(file_p)->close() ? LV_FS_RES_OK : LV_FS_RES_FS_ERR;
    delete SD_FILE(file_p);
#if SD_USE_SEM
    xSemaphoreGive(sdCardSemaphore);
#endif
    return res;
}

/**
 * Read data from an opened file
 * @param drv pointer to a driver where this function belongs
 * @param file_p pointer to a file_t variable.
 * @param buf pointer to a memory block where to store the read data
 * @param btr number of Bytes To Read
 * @param br the real number of read bytes (Byte Read)
 * @return LV_FS_RES_OK: no error, the file is read
 *         any error from lv_fs_res_t enum
 */

static lv_fs_res_t fs_read (lv_fs_drv_t * drv, void * file_p, void * buf, uint32_t btr, uint32_t * br)
{
#if SD_USE_SEM
    if (xSemaphoreTake(sdCardSemaphore, 10) != pdPASS)
    {
        LV_LOG_ERROR("sdCardSemaphore take failed in fs_read");
        return NULL;
    }
    markSemaphore(FUNCTION_LVGL_READ);
#endif
    int ret = SD_FILE(file_p)->read(buf, btr);

    if(ret < 0)
    {
#if SD_USE_SEM
        xSemaphoreGive(sdCardSemaphore);
#endif
        return LV_FS_RES_FS_ERR;
    }

    *br = ret;

#if SD_USE_SEM
    xSemaphoreGive(sdCardSemaphore);
#endif
    return LV_FS_RES_OK;
}

/**
 * Write into a file
 * @param drv pointer to a driver where this function belongs
 * @param file_p pointer to a file_t variable
 * @param buf pointer to a buffer with the bytes to write
 * @param btr Bytes To Write
 * @param br the number of real written bytes (Bytes Written). NULL if unused.
 * @return LV_FS_RES_OK or any error from lv_fs_res_t enum
 */
static lv_fs_res_t fs_write(lv_fs_drv_t * drv, void * file_p, const void * buf, uint32_t btw, uint32_t * bw)
{
#if SD_USE_SEM
    if (xSemaphoreTake(sdCardSemaphore, 10) != pdPASS)
    {
        LV_LOG_ERROR("sdCardSemaphore take failed in fs_write");
        return NULL;
    }
    markSemaphore(FUNCTION_LVGL_WRITE);
#endif
    int ret = SD_FILE(file_p)->write((const uint8_t*)buf, btw);

    if(ret < 0)
    {
#if SD_USE_SEM
        xSemaphoreGive(sdCardSemaphore);
#endif
        return LV_FS_RES_FS_ERR;
    }

    *bw = ret;

#if SD_USE_SEM
    xSemaphoreGive(sdCardSemaphore);
#endif
    return LV_FS_RES_OK;
}

/**
 * Set the read write pointer. Also expand the file size if necessary.
 * @param drv pointer to a driver where this function belongs
 * @param file_p pointer to a file_t variable. (opened with lv_ufs_open )
 * @param pos the new position of read write pointer
 * @return LV_FS_RES_OK: no error, the file is read
 *         any error from lv_fs_res_t enum
 */
static lv_fs_res_t fs_seek (lv_fs_drv_t * drv, void * file_p, uint32_t pos, lv_fs_whence_t whence)
{
#if SD_USE_SEM
    if (xSemaphoreTake(sdCardSemaphore, 10) != pdPASS)
    {
        LV_LOG_ERROR("sdCardSemaphore take failed in fs_seek");
        return NULL;
    }
    markSemaphore(FUNCTION_LVGL_SEEK);
#endif
    if(whence == LV_FS_SEEK_SET)
    {
        SD_FILE(file_p)->seekSet(pos);
    }
    else if(whence == LV_FS_SEEK_CUR)
    {
        SD_FILE(file_p)->seekCur(pos);
    }
    else if(whence == LV_FS_SEEK_END)
    {
        SD_FILE(file_p)->seekEnd();
    }
    else
    {
#if SD_USE_SEM
        xSemaphoreGive(sdCardSemaphore);
#endif
        return LV_FS_RES_UNKNOWN;
    }

#if SD_USE_SEM
    xSemaphoreGive(sdCardSemaphore);
#endif
    return LV_FS_RES_OK;
}

/**
 * Give the position of the read write pointer
 * @param drv pointer to a driver where this function belongs
 * @param file_p pointer to a file_t variable.
 * @param pos_p pointer to to store the result
 * @return LV_FS_RES_OK: no error, the file is read
 *         any error from lv_fs_res_t enum
 */
static lv_fs_res_t fs_tell (lv_fs_drv_t * drv, void * file_p, uint32_t * pos_p)
{
#if SD_USE_SEM
    if (xSemaphoreTake(sdCardSemaphore, 10) != pdPASS)
    {
        LV_LOG_ERROR("sdCardSemaphore take failed in fs_tell");
        return NULL;
    }
    markSemaphore(FUNCTION_LVGL_TELL);
#endif
    *pos_p = SD_FILE(file_p)->curPosition();
#if SD_USE_SEM
    xSemaphoreGive(sdCardSemaphore);
#endif
    return LV_FS_RES_OK;
}

/**
 * Initialize a 'fs_read_dir_t' variable for directory reading
 * @param drv pointer to a driver where this function belongs
 * @param dir_p pointer to a 'fs_read_dir_t' variable
 * @param path path to a directory
 * @return LV_FS_RES_OK or any error from lv_fs_res_t enum
 */
static void * fs_dir_open(lv_fs_drv_t * drv, const char * path)
{
#if SD_USE_SEM
    if (xSemaphoreTake(sdCardSemaphore, 10) != pdPASS)
    {
        LV_LOG_ERROR("sdCardSemaphore take failed in fs_dir_open");
        return NULL;
    }
    markSemaphore(FUNCTION_LVGL_DIR_OPEN);
#endif
    rddir_t * dir_p = new rddir_t;

    if(dir_p == NULL)
    {
#if SD_USE_SEM
        xSemaphoreGive(sdCardSemaphore);
#endif
        return NULL;
    }

    if(!dir_p->open(path))
    {
        delete dir_p;
        dir_p = NULL;
    }
#if SD_USE_SEM
    xSemaphoreGive(sdCardSemaphore);
#endif
    return dir_p;
}

/**
 * Read the next filename form a directory.
 * The name of the directories will begin with '/'
 * @param drv pointer to a driver where this function belongs
 * @param dir_p pointer to an initialized 'fs_read_dir_t' variable
 * @param fn pointer to a buffer to store the filename
 * @return LV_FS_RES_OK or any error from lv_fs_res_t enum
 */
static lv_fs_res_t fs_dir_read (lv_fs_drv_t * drv, void * dir_p, char *fn)
{
#if SD_USE_SEM
    if (xSemaphoreTake(sdCardSemaphore, 10) != pdPASS)
    {
        LV_LOG_ERROR("sdCardSemaphore take failed in fs_dir_read");
        return NULL;
    }
    markSemaphore(FUNCTION_LVGL_DIR_READ);
#endif
    file_t file;
    char name[128];

    do
    {
        if(file.openNext(SD_DIR(dir_p), O_RDONLY))
        {
            file.getName(name, sizeof(name));

            if(file.isDir())
            {
                fn[0] = '/';
                strcpy(&fn[1], name);
            }
            else
            {
                strcpy(fn, name);
            }

            file.close();
        }
        else
        {
            fn[0] = '\0';
        }
    }while(strcmp(fn, "/.") == 0 || strcmp(fn, "/..") == 0);

#if SD_USE_SEM
    xSemaphoreGive(sdCardSemaphore);
#endif
    return LV_FS_RES_OK;
}

/**
 * Close the directory reading
 * @param drv pointer to a driver where this function belongs
 * @param dir_p pointer to an initialized 'fs_read_dir_t' variable
 * @return LV_FS_RES_OK or any error from lv_fs_res_t enum
 */
static lv_fs_res_t fs_dir_close (lv_fs_drv_t * drv, void * dir_p)
{
#if SD_USE_SEM
    if (xSemaphoreTake(sdCardSemaphore, 10) != pdPASS)
    {
        LV_LOG_ERROR("sdCardSemaphore take failed in fs_dir_close");
        return NULL;
    }
    markSemaphore(FUNCTION_LVGL_DIR_CLOSE);
#endif
    lv_res_t res = SD_DIR(dir_p)->close() ? LV_FS_RES_OK : LV_FS_RES_FS_ERR;
    delete SD_DIR(dir_p);
#if SD_USE_SEM
    xSemaphoreGive(sdCardSemaphore);
#endif
    return res;
}