#ifdef __cplusplus
extern "C" {
#endif

void serial_init(void);
void serial_open(unsigned int baud_rate);
void serial_close(void);
void serial_write(char ch);

#ifdef __cplusplus
}  /* extern "C" */
#endif
