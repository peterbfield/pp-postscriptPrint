#ifndef _PSJOB_F
#define _PSJOB_F

/* from shared or system routines */

void cleanup(int sig);
void clean_wysiwyg(WYSIWYG *wn);
void exit(int);
int frame_rd(WYSIWYG *wn, int dir_type, char *filename);
char *itoa(int, char *);
int lmt_interface(WYSIWYG *wn);
int32 lmt_off_to_abs(WYSIWYG *wn, int type, uint32 value);
void lmt_page_elements(WYSIWYG *wn);
int map_load(WYSIWYG *wn, char *filename, QUEUE *map_data, 
			 int layout_text_flag);
int read_line_def(LDEF *fo_line_def, int16 (*foget) () );
void set_sigs();


/* from pp */

void add_color(void);
int add_font(char *font, int mask, F_LIST **list);
void add_to_vlist(float x, float y, float x2, float y3, int fg, int fg_shd,
                  int bg, int bg_shd, uint32 vlist_plates, int bf_flag);

void beg_JOB(char *new);
void beg_PAGE(void);

void camy(int16 kld, char *mes_str);
void clear_flist(F_LIST **lst);
void clear_vlist(void);
int color_check(int color, int def);
void color_func(int in_color, int in_shade);
void create1psfont(char *psfname, int psfps, int psfss, float obdegree_tmp);

void dbox(int fill_or_stroke_flag, int bf_radius_flag);
void dbox_sub(float x1, float y1, float x2, float y2);
void delay_records(void);
void digi_print(float num);
void fdigi_print(float num);
void do_pskmd(char action, char *cmd_str);
void drule(void);
void drule_sub(int stroke_flag);
void dfrule(void);
int dtext(void);
int dxy(void);

void end_JOB(void);
void end_PAGE(void);
void error(char *s1, char *s2, int eror);

struct clr_lst *find_color(int color, uint32 plate_number);
void find_mu_rot(int i);
int16 foget(void);
void foread(int16 recnum);

void getftab(void);
void get_next_piece(void);
void getpgsize(void);
void getprolog(void);
char *get_ps_fname(int article_id);
void graphic(int16 fo_cmd);

void ini_JOB(void);
void ini_PAGE(void);
void init_color(void);
void init_mpu_in_use_print(void);
void init_mpu_lprint (void);
void init_mpu_txprint (void);

void kfolio(int ival);
int kmd0();
int kmd1();
int kmd2();
int kmd3();
int kmd4();
int kmd5();
int kmd6();
int kmd7();
int kmd8();
int kmd9();
int kmd10();
int kmd11();
int kmd12();
int kmd13();
int kmd14();
int kmd15();
int kmd16();
int kmd17();
int kmd18();
int kmd19();
int kmd20();
int kmd21();
int kmd22();
int kmd23();
int kmd24();
int kmd25();
int kmd26();
int kmd27();
int kmd28();
int kmd29();
int kmd30();
int kmd31();
int kmd32();
int kmd33();
int kmd34();
int kmd35();
int kmd36();
int kmd37();
int kmd38();
int kmd39();
int kmd40();
int kmd41();
int kmd42();
int kmd43();
int kmd44();
int kmd45();
int kmd46();
int kmd47();
int kmd48();
int kmd49();
int kmd50();
int kmd51();
int kmd52();
int kmd53();
int kmd54();
int kmd55();
int kmd56();
int kmd57();
int kmd58();
int kmd59();
int kmd60();
int kmd61();
int kmd62();
int kmd63();
int kmd64();
int kmd65();
int kmd66();
int kmd67();
int kmd68();
int kmd69();
int kmd70();
int kmd71();
int kmd72();
int kmd73();
int kmd74();
int kmd75();
int kmd76();
int kmd77();
int kmd78();
int kmd79();
int kmd80();
int kmd81();
int kmd82();

void list_fonts(F_LIST **list, char *font_set, int mode);
int log_font(char *font, int mask);

void m_clear(void);
void m_close(int psfm);
int m_fopen(char *fnm, char * mde, int psfm);
void m_fprintf(char* form, ...);
void mpu_data_print(void);
void mpu_data_print_br(void);
void mpu_data_print_ch(void);
void mpu_data_print_fn(void);
void mpu_data_print_il(void);
void mpu_data_print_sn(void);
void mpu_data_print_vs(void);
void mpu_layout_print(int frame, ELEMENT *ele);
void m_putc(unsigned char bite);
void mpu_textfile_print(int header_flag);
void m_tran_ov(char *of);

void output_ele_xy(float x, float y);
void output_mpu_lprint (void);
void output_mpu_post_report (void);
void output_mpu_txprint (void);

int psbuild(int page);
void ps_drawline(void);
void psmpu_main(void);
int psinit(void);
void ps_set_rotation(void);
void put_font(void);
void put_lnum(void);

void resend_kmd_bf(int16 gal_depth);
void return_mpu_que(void);

void scan_map(int layout_num);
void send_it(char buff[200][132], int *next_line);
void set_edit_trace(int level);
void set_edit_trace_add(int level);
void set_edit_trace_del(int level);
void set_layout_process_flag(LAYOUT_DESC *layout);
void set_pass_2_color(int color, uint32 *mask_pointer, uint32 start_plate);
void set_under_score(void);
int skmd8();
int skmd13();
int skmd14();
int skmd15();
int skmd19();
int skmd20();
int skmd21();
int skmd22();
int skmd23();
int skmd24();
void spot_concat(void);
void stack(uchar khar);
void stack_print(void);
void stop(char *s1, char *s2, int eror);

int tp_open(void);
void trace_idtape(struct ps_pieces *current_rec);

int verify_and_add_mpu_name (int index);
struct vid_box *vlist_intersect(uint32 plate_number);
void vid_color(int color, int shade, uint32 plate_number);

void width_open(void);
void width_read(void);

#endif

