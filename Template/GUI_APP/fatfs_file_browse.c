
#include "fatfs_file_browse.h"
#include "ff.h"

static FATFS   fs;
static FRESULT fr;
static DIR     dir;
static DIR     DirInfo;
static FILINFO fileinfo;    //文件信息
char file_name[FILE_NUM][NAME_LENGTH];//文件名
char folder_name[FOLDER_NUM][NAME_LENGTH];//文件夹名
uint8_t file_cnt = 0;
uint8_t folder_cnt = 0;

Folder_btnArray *p;
File_btnArray *q;

uint8_t current_path[PATH_LENGTH] = "0:"; //modify spiflash  "1:" ,SD_CARD "0"
char mountsel[] = "0:"; //modify spiflash  "1:" ,SD_CARD "0"

extern void event_handler(lv_obj_t *obj, lv_event_t event);
extern void lv_ex_msgbox_1(void);
extern void file_btnevent_handler(lv_obj_t *obj, lv_event_t event);
extern void filelist_btnevent_handler(lv_obj_t *obj, lv_event_t event);

#define ACTIVE_ALL_FILE 1

//遍历文件 显示到屏幕
uint8_t mf_scan_file(char *path)
{
    int i, j;
    FRESULT res;
    char *fn;   /* This function is assuming non-Unicode cfg. */
    deinit_list();
    res = f_opendir(&dir, (const TCHAR *)path); //打开一个目录
    if(res == FR_OK) {
        while(1) {
            res = f_readdir(&dir, &fileinfo);                   //读取目录下的一个文件
            if(res != FR_OK || fileinfo.fname[0] == 0) {
                break;    //错误了/到末尾了,退出
            }
            if(fileinfo.fattrib & AM_ARC) {                    /* 读取的是文件名字 */
                fn = *fileinfo.altname ? fileinfo.fname : fileinfo.fname;
#if  ACTIVE_ALL_FILE
                sprintf(file_name[file_cnt], "%s", fn); //拼接路径名
                file_cnt++;
                if(file_cnt >= FILE_NUM) {
                    break;
                }
#endif
#if  !ACTIVE_ALL_FILE
                if(strstr(fileinfo.fname, ".bin")) {
                    sprintf(file_name[file_cnt], "%s", fn); //拼接路径名
                    file_cnt++;
                    if(file_cnt >= FILE_NUM) {
                        break;
                    }
                } else {
                    break;
                }
#endif
            } else if(fileinfo.fattrib & AM_DIR) {       /* 读取的是文件夹名字 */
                fn = *fileinfo.altname ? fileinfo.fname : fileinfo.fname;
                sprintf(folder_name[folder_cnt], "%s", fn); //拼接路径名

                folder_cnt++;
                if(folder_cnt >= FOLDER_NUM) {
                    break;
                }
            }
        }
    }
    return res;
}

void deinit_list(void)
{
    file_cnt = 0;
    folder_cnt = 0;
    memset(file_name, 0, FILE_NUM * NAME_LENGTH * sizeof(char));
    memset(folder_name, 0, FOLDER_NUM * NAME_LENGTH * sizeof(char));
    free(p);
    free(q);
    p = NULL;
    q = NULL;
}

void deinit_list2(void)
{
    file_cnt = 0;
    folder_cnt = 0;
    memset(file_name, 0, FILE_NUM * NAME_LENGTH * sizeof(char));
    memset(folder_name, 0, FOLDER_NUM * NAME_LENGTH * sizeof(char));
    memset(current_path, 0, sizeof(current_path));
    strncpy((char *)(current_path), mountsel, 3);  //modify spiflash  "1:"
    printf("deinit_list: current path is %s\r\n", (char *)(current_path));
    free(p);
    free(q);
}

uint8_t get_current_path(void)
{
    char *p1 = NULL;
    uint8_t length, i;
    uint8_t tmp_path[PATH_LEN];
    memset(tmp_path, 0, PATH_LEN * sizeof(uint8_t));

    /* 获取当前路径，存储到tmp_path */
    for(i = 0; i < PATH_LEN; i++) {
        tmp_path[i] = current_path[i];
    }
    /* strrchr：查找字符串'/',从后往前的第一个出现的位置 */
    p1 = strrchr((char *)(current_path), '/');
    length = (p1 - (char *)(current_path)) / (sizeof(char));
    memset(current_path, 0, PATH_LEN * sizeof(uint8_t));
    /* 将tmp_path中前length个字节复制到current_path */
    strncpy((char *)(current_path), (char *)tmp_path, length);
    if(strcmp(mountsel, (char *)(current_path)) != 0) {  //modify spiflash  "1:"
        return 1;
    } else {
        return 0;
    }
}

void refersh_parameter(void)
{
    get_current_path();
    deinit_list();
    mf_scan_file((char *)(current_path));
}


void scanfiles_test()
{
    fr = f_mount(&fs, "0:", 1);
}
