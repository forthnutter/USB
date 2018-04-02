#ifndef rawhid_included_h__
#define rawhid_included_h__

// Raw HID, Basic API
typedef void rawhid_t;
rawhid_t * rawhid_open_only1(int vid, int pid, int usage_page, int usage);
int rawhid_status(rawhid_t *hid);
int rawhid_read(rawhid_t *h, void *buf, int bufsize, int timeout_ms);
int rawhid_write(rawhid_t *hid, const void *buf, int len, int timeout_ms);
void rawhid_close(rawhid_t *h);


// Raw HID, Multiple Device API
typedef void rawhid_list_t;
rawhid_list_t * rawhid_list_open(int vid, int pid, int usage_page, int usage);
int rawhid_list_count(rawhid_list_t *list);
void rawhid_list_close(rawhid_list_t *list);
int rawhid_list_indexof(rawhid_list_t *list, rawhid_t *hid);
void rawhid_list_remove(rawhid_list_t *list, rawhid_t *hid);
rawhid_t * rawhid_open(rawhid_list_t *list, int index);


#endif
