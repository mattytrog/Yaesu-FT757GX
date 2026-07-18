#ifdef include_cb

int8 cb_channel;
int8 cb_region;

#endif

int32 band_bank[11] = 
{
   50000, 180000, 350000, 700000, 1010000,
   1400000, 1800000, 2100000, 2400000, 2800000, 3000000
};

#ifdef include_pms
int32 band_bank_edge[11] = 
{
   160000, 200000, 380000, 720000, 1020000,
   1435000, 1820000, 2145000, 2500000, 3000000, 3000000
};
#endif

int32 PLL_band_bank[30] = 
{
   //lower limit, upper limit, result
   0, 149999, 1,
   150000, 249999, 1,
   250000, 399999, 2,
   400000, 749999, 3,
   750000, 1049999, 4,
   1050000, 1449999, 5,
   1450000, 1849999, 6,
   1850000, 2149999, 7,
   2150000, 2499999, 8,
   2500000, 9999999, 9
};

//VAL1  VAL2
//>VAL1 && <VAL2 = OK
//<VAL1 && >VAL2 = Out of band
int32 blacklist[20]= 
{
   150000, 200000,
   350000, 400000,
   700000, 750000,
   1000000, 1050000,
   1400000, 1450000,
   1800000, 1850000,
   2100000, 2150000,
   2400000, 2550000,
   2800000, 3000000
};

char disp_buf[13] = {10,0,15,15,15,15,15,15,15,15,15,15,15};
int8 dcs_res[32] = 
{
   //dcs dl cl sl
   15, 0, 0, 0,
   14, 0, 1, 1,
   12, 0, 0, 1,
   4, 1, 1, 1,
   3, 1, 1, 0,
   2, 0, 1, 0,
   1, 1, 0, 0,
   0, 1, 0, 1
};

#ifdef include_cat
const char cat_comm[200] =
{

   //1st char, 2nd char, location of terminator, res. eg A is 0, I is 1, 0 or 1 would be 2, terminator at 3, result is function 1.
   'I', 'D', 2, 1, // ID = READ. ANSWER.  Answer (ID006;)
   'A', 'I', 3, 2,  //AI0 = SET. NO ANSWER. IF off. AI1 = SET. ANSWER. IF on. Answer IF
   'D', 'N', 2, 3,  // DN = SET. NO ANSWER. DOWN. No reply. Action up button
   'U', 'P', 2, 4,  // UP = SET. NO ANSWER. UP. No reply. Action down button
   'F', 'A', 2, 5,  // FA = READ. ANSWER. VFOA Answer (FA000) (Freq) (0) (;)
   'F', 'B', 2, 6,  // FB = READ. ANSWER. VFOB Answer (FB000) (Freq) (0) (;)
   'F', 'A', 13, 7, // FA = SET. ANSWER. VFOA Set & Answer (FA000) (freq) (0) (;)
   'F', 'B', 13, 8, // FB = SET. ANSWER. VFOB Set & Answer (FB000) (freq) (0) (;)
   'F', 'N', 3, 9,  // FN = SET. NO ANSWER. (0;) = VFOA, (1;) = VFOB, (2;) = MR,
   'F', 'R', 3, 10,  // FR = SET. ANSWER. (FR) (0;) = VFOA, (1;) = VFOB, Answer FR (VFO)(;)
   'F', 'R', 2, 11,  // FR = READ. ANSWER. (FR) (0;) = VFOA, (1;) = VFOB
   'F', 'T', 3, 12,  // FT = SET. ANSWER. (FT) (0;) = VFOA, (1;) = VFOB, Answer FT (VFO)(;)
   'F', 'T', 2, 13,  // FT = READ. ANSWER. (FT) (0;) = VFOA, (1;) = VFOB
   'I', 'F', 2, 14, // IF = READ. ANSWER. Answer IF
   'I', 'E', 3, 15, // IW = *CUSTOM SET Rig ID
   'L', 'K', 2, 16, // LK = READ. ANSWER. Answer LK0; or LK1; UNLOCK OR LOCK
   'L', 'K', 3, 17, // LK = SET. ANSWER. LK0; Lock off. LK1; Lock on....Answer LK0; / LK1; UNLOCK/LOCK
   'M', 'C', 5, 18, // MC = SET. NO ANSWER. MC Memory channel. (MC)(0)(CH). eg MC002; = mem 2
   'M', 'D', 3, 19, // MD = SET. NO ANSWER. MD; MODE - Fake mode 1 = LSB, 2 = USB, 3 = CW, 4 = FM, 5 = AM, 7 = CWN
   'M', 'R', 6, 20, // MR = READ. ANSWER. (MR) (0) (0) (memch) (;). ANSWER (MR) (0) (0) (mem ch. 2 digits) (000) (Frequency. + 0) (dummy mode) (0) (0) (00) (0) ;
   'M', 'W', 23, 21,// MW = SET. NO ANSWER (MW) (0) (0) (mem ch. 2 digits) (000) (Frequency. + 0) (dummy mode) (0) (0) (00) (0) ;

   'R', 'C', 2, 22, // RC = SET. NO ANSWER. Clarifier offset = 0.
   'R', 'D', 2, 23, // RD = SET. NO ANSWER. Clarifier freq decrease.
   'R', 'U', 2, 24, // RU = SET. NO ANSWER.  Clarifier freq increase.
   'R', 'T', 3, 25, // RT = SET. NO ANSWER. RT0; = Clar off. RT1 = Clar on.
   'R', 'X', 2, 26, // RX = SET. NO ANSWER. RX; mode.
   'T', 'X', 2, 27, // TX = SET. NO ANSWER. TX; mode.
   'S', 'C', 3, 28, // SC = SET. NO ANSWER. SC0; PMS off...SC1; PMS on.
   'S', 'P', 3, 29, // SP = SET. NO ANSWER. SP0; split off...SP1 split on.
   'Y', 'A', 2, 30, // SP = SET. NO ANSWER. SP0; split off...SP1 split on.
   //!
};

//create large buffer, based on longest answer. Everything after the terminator is nulled. Buffer is always the same length
char cat_ans[24] = 
{
   '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0'
};
//char* cat_ptr = &cat_ans[];

//38 char IF buffer. For Kenwood information request. Elements are swapped as needed
char ifbuf[38] = 
{
   'I', 'F', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0',
   '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', ';'
};

char idbuf[5] = 
{
   'I', 'D', '0', '0', '6'
};

#endif



