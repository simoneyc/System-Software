# include <stdio.h>
# include <stdlib.h>
# include <string.h>
// file variable
FILE *source_pass1,*source_pass2,*object_pro,*symbol_TAB,*source_code,*opCode;
int Length,top = 0,symTab_size = 2, loc, start_address=0;
// define op_table from 'opcode'
// 儲存input opcode 的 data 
// add_queue 18
// ADDF 58
// ADDR 90
// AND 40
// CLEAR B4
// ...
typedef struct opcodeStruct opcodeStruct;
struct opcodeStruct{
	char text[8];
	char op_num[3];
	struct opcodeStruct* next;
};
opcodeStruct **op_table;
opcodeStruct* check_opcodeTab(char*); 

// define symbol_table
// 長這樣: 
// FIRST	1000
// CLOOP	1003
// ENDFIL	1015
// (text)   (address)
// ...
typedef struct symbolStruct symbolStruct;
struct symbolStruct{
	char text[16];
	int address;
};
symbolStruct* symbol_table;

// set op_table with opcode
// 把opcode存好 
void opcode_setting(opcodeStruct* pointer, char* readline){ 
	pointer->next = NULL;
	// 以space間隔: 'add_queue 18'
	int i;
	for(i = 0;readline[i] != ' ';i++){
		pointer->text[i] = readline[i];
	}
	// 讀'\0' 16進位要轉換 
	pointer->text[i] = '\0';
	pointer->op_num[0] = readline[++i];
	pointer->op_num[1] = readline[++i];
	pointer->op_num[2] = '\0';
}

// input = strbuff , generate data
void get_buff(char* str_buff,char* symbol_buff,char* opcode_setting,char* Input_buff){
//	RETADR	RESW	1
//	LENGTH	RESW	1
//	BUFFER 	RESB	4096
//	RDREC	LDX		ZERO
//  ...

	// symbolbuff => 'FIRST'	STL	RETADR
	int i = 0,j;
	for(j = 0; str_buff[i] != '\t'&&str_buff[i] != ' '; i++,j++){
		symbol_buff[j] = str_buff[i];
	}
	symbol_buff[j] = '\0';
	// 跳過 space or tab
	for(;;i++){ 
		if((str_buff[i] != '\t'&&str_buff[i] != ' ')&&(str_buff[i-1] == '\t'||str_buff[i-1] == ' ')){
			break;
		}
	}
	// opcodebuff => FIRST	'STL'	RETADR
	for(j = 0;str_buff[i] != '\t'&&str_buff[i] != ' '&&str_buff[i] != '\0';i++,j++){
		opcode_setting[j] = str_buff[i];
	}
	opcode_setting[j] = '\0';
	// 跳過 space or tab
	for(;str_buff[i] != '\0';i++){
		if((str_buff[i] != '\t'&&str_buff[i] != ' ')&&(str_buff[i-1] == '\t'||str_buff[i-1] == ' ')){
			break;
		}
	}
	// inputbuff => FIRST	STL	'RETADR'
	for(j = 0;str_buff[i] != '\t'&&str_buff[i] != ' '&&str_buff[i] != '\0';i++,j++){ 
		Input_buff[j] = str_buff[i];
	}
	Input_buff[j] = '\0';	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//**pass1 開始**																													//
//pass1 任務有: 																													//
//Calculate the loc value 																											//
//Create the symbol table 																											//
//Generate the immediate file																										//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// pass1

// stack-push function
// 建立stack用以創建symbol table 
void push(char* symbol_buff){
	int i;
	for(i = 0;i < top;i++){
		if(!stricmp((symbol_table+i)->text,symbol_buff)){
			printf("error with symbol\n");
			exit(0);
		}
	}
	symTab_size = 1000;   
	symbolStruct* temp = (symbolStruct*)malloc(symTab_size*sizeof(symbolStruct));
	for(i = 0;i < top;i++){
		strcpy((temp+i)->text,(symbol_table+i)->text); // copy src to dest (dest, src) , return dest 
		(temp+i)->address = (symbol_table+i)->address;
	}
	symbol_table = temp;
	
	
	strcpy((symbol_table+top)->text,symbol_buff);
	(symbol_table+top)->address = loc;
	top++;
}

// 產出有loc value的 source program
// 這邊生成 pass1's source_program
void do_pass1_source(){
	source_pass1 = fopen("pass1_source_program.txt","w");
	char str_buff[200]; // symbol opcode
	char c_buff;
	char symbol_buff[50];
	char opcode_setting[50];
	char Input_buff[50];
	// 分配memory給symbol table 
	symbol_table = (symbolStruct*)malloc(symTab_size*sizeof(symbolStruct));
	// 讀, if 遇到 comment 則往下
	do{
		fscanf(source_code,"%[^\n]",str_buff);
		c_buff = fgetc(source_code); // 暫存"\n"   
	}while(commentLine(str_buff)); // 略過註解 
	
	// 抓個別data 
	get_buff(str_buff,symbol_buff,opcode_setting,Input_buff);
	int i;
	
	// 找 START 之後的 line 
	if(!stricmp(opcode_setting,"START")){
		start_address = s16_To_Int(Input_buff);
		loc = start_address;
		// 逐行 print 出 
		fprintf(source_pass1,"%X\t%s\n",loc,str_buff);
	}
	else{
		// START 那行 LOC 設 0 
		loc = 0;
	}
	
	// 與前述類似,跳過 comment  
	do{
		fscanf(source_code,"%[^\n]",str_buff); 
		c_buff = fgetc(source_code);
	}while(commentLine(str_buff));
	get_buff(str_buff,symbol_buff,opcode_setting,Input_buff);
	   
	// END 為終止訊號 
	while(stricmp(opcode_setting,"END")){ // END 指示原始程式的結束處 並指定程式中第一個可執行的指令
		// print
		fprintf(source_pass1,"%X\t%s\n",loc,str_buff);
		
		// push 去生出symbolTAB 
		if(symbol_buff[0] != '\0'){
			push(symbol_buff);
		}
		// 計算下一行的 loc (用opcode) 
		// BYTE,WORD是轉換位元or字組指令,RESB,RESW保留記憶體空間
		if(!stricmp(opcode_setting,"BYTE")){ // 定義字元或十六進位的常數 並且指出其可佔用之位元組的數量
			if(Input_buff[0] == 'X'){ // 十六進位數值 ASCII 
				loc += 1;
			}
			else if(Input_buff[0] == 'C'){ // 字串 +3對齊  
				loc += 3;
			} 
		}
		else if(!stricmp(opcode_setting,"WORD")){ // 定義一個字組的整數常數
			loc += 3;
		}
		else if(!stricmp(opcode_setting,"RESB")){ // RESB 保留所示數量的位元組 供資料區使用
			int cnt = s10_To_Int(Input_buff);
			loc += cnt;
		}
		else if(!stricmp(opcode_setting,"RESW")){ // RESW 保留所示數量的字組 供資料區使用
			int cnt = s10_To_Int(Input_buff);
			loc += (3*cnt); // RESW 是 WORD
		}
		// 查表 
		else{
			opcodeStruct* pointer = check_opcodeTab(opcode_setting);
			if(pointer == NULL){
				// 報錯 
				printf("error with opcode\n");
				exit(0);
			}
			loc += 3;
		}
		
		// 讀下一行 
		do{
			fscanf(source_code,"%[^\n]",str_buff);
			c_buff = fgetc(source_code);
		}while(commentLine(str_buff));
		get_buff(str_buff,symbol_buff,opcode_setting,Input_buff);
	}
	
	//	print END line
	fprintf(source_pass1,"\t%s\n",str_buff);
	Length = loc-start_address; // 程式總長度 END - START 
	fclose(source_pass1);
	printf("\'pass1_source_program.txt\' 	has been generated!\n\n"); // pass1_source_program.txt 是 pass1 有包含 loc 的 txt 但還沒包含 object code 
}

// generate - SymbolTable 寫入 file 
void do_SymbolTable(){
	symbol_TAB = fopen("symbol_table.txt","w");
	int i;
	for(i = 0;i < top;i++){
		fprintf(symbol_TAB,"%s\t%X\n",(symbol_table+i)->text,(symbol_table+i)->address); // symbol_table記憶體處+i來尋找symbol table的 data 
	}
	fclose(symbol_TAB);
	printf("\'symbol_table.txt\' 		has been generated!\n\n"); // 這裡生成symbol table 
}

// check if 'comment line'
int commentLine(char* Line){
	int i;
	for(i = 0;i < strlen(Line);i++){
		if(Line[i] != ' '&&Line[i] != '\t')	
			break;
	}
	// comment 是 . 
	if(Line[i] == 46){
		return 1;
	}
	else{
		return 0;
	}
}

// (Hex in str) to Int : s16 -> 10
int s16_To_Int(char* Input_buff){ // 輸入參數'Input_buff'指向字符數組
	int ten = 0; // ten = 0 用於存最後的十進值
	int count = strlen(Input_buff) - 1; 
	int i,j;
	for(i = 0;Input_buff[i] != '\0';i++,count--){
		int numbuff = 0;
		if(Input_buff[i] >= '0'&&Input_buff[i] <= '9'){
			numbuff = Input_buff[i] - '0';
		}
		else if(Input_buff[i] >= 'A'&&Input_buff[i] <= 'F'){
			numbuff = Input_buff[i] - 'A' + 10;
		}
		for(j = 0;j < count;j++){
			numbuff *= 16;
		}
		ten += numbuff;	
	}
	return ten;
}

// (Int in str) to Int : s10 -> 10
int s10_To_Int(char* Input_buff){
	int ten = 0; // ten = 0 用於存最後的十進值
	int count = strlen(Input_buff) - 1;
	int i,j;
	for(i = 0;Input_buff[i] != '\0';i++,count--){
		int numbuff = 0;
		numbuff = Input_buff[i] - '0';
		for(j = 0;j < count;j++){
			numbuff *= 10;
		}
		ten += numbuff;	
	}
	return ten;
}

// find opCode in op_table 查表 
// return LOC
// 查 opcode's function 
opcodeStruct* check_opcodeTab(char* opcode_setting){ // 參數opcode_setting表示要搜索的opcode是誰 
	opcodeStruct* pointer;
	if(opcode_setting[0] >= 'A'&&opcode_setting[0] <= 'Z'){
		pointer = *(op_table + opcode_setting[0]-'A');// 計算出index，該函數就會檢索指向opCodeUnit該index處Link中第一個pointer
		while(pointer != NULL){
			if(!stricmp(pointer->text,opcode_setting)){  // 找到了! break 去 return 那個 pointer指向的傢伙 if (s1 == s2) , return 0
				break;
			}
			pointer = pointer->next; // 往下找
		}
	}
	else{
		printf("error with opcode\n"); // print if error exist
		exit(0);
	}
	return pointer; // return pointer
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//**pass2 開始**																													//
//pass2 任務有: 																													//
//透過計算object code得到最後的object program 																						//
//所以也把加入object code後的source fprintf出來 																					//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// 建立queue 
// 生成object program用 
int rear = 0;
typedef struct link link;
struct link{
	char ob_first[4];
	int data_location;
	int locat;
	int add_queue;
	char flag;
};
link* queue;

void add_queue(char* ob_first,int data_location,int locat,int add_queue,char flag){
	strcpy((queue+rear)->ob_first,ob_first);
	(queue+rear)->data_location = data_location;
	(queue+rear)->locat = locat;
	(queue+rear)->add_queue = add_queue;
	(queue+rear)->flag = flag;
	rear++;
}
char pgname[50];

// pass 2 -  SourceProgram 生成
void do_pass2_source(){
	source_pass2 = fopen("pass2_source_program.txt","w");
	char str_buff[100];
	char c_buff;
	char symbol_buff[16];
	char opcode_setting[8];
	char Input_buff[16];
	char ob_first[4];
	opcodeStruct* pointer;
	int objcode = 0;
	int ob_second;
	// object_code 分前後 前面first=symbol查到的數字 second=(symbol_table+index)的address 
	 
	// 先處理第一行 COPY	START	1000
	do{
		fscanf(source_code,"%[^\n]",str_buff);
		c_buff = fgetc(source_code);
	}while(commentLine(str_buff)); // ignore comments 
	
	get_buff(str_buff,symbol_buff,opcode_setting,Input_buff);
	strcpy(pgname,symbol_buff); // program name = first symbol_buff 抓到的傢伙 
	// first line
	loc = start_address;
	fprintf(source_pass2,"%X\t%s\n",loc,str_buff);
	// 處理第一行以後的傢伙們 
	do{
		fscanf(source_code,"%[^\n]",str_buff);
		c_buff = fgetc(source_code);
	}while(commentLine(str_buff));// ignore comments 
	
	get_buff(str_buff,symbol_buff,opcode_setting,Input_buff);
	// malloc memory 給 queue 
	queue = (link*)malloc(sizeof(link)*1000);
	
	while(stricmp(opcode_setting,"END")){
		pointer = check_opcodeTab(opcode_setting); // find opcode
		if (pointer == NULL){ // 找謀的話 NULL (特殊的) 不在symbol table裡面的 
			strcpy(ob_first,"");  // 複製空的的對應碼到 ob_first 
			if(!stricmp(opcode_setting,"BYTE")){  //stricmp 相等 : = 0
				// BYTE 有兩種 
				if(Input_buff[0] == 'X'){ // EX: X'F1'
					char bufX16[3];
					bufX16[0] = Input_buff[2];
					bufX16[1] = Input_buff[3];
					bufX16[2] = '\0';
					fprintf(source_pass2,"%x\t%s\t%s\t%s\t\t%s%02X\n",loc,symbol_buff,opcode_setting,Input_buff,ob_first,s16_To_Int(bufX16));
					// into queue
					add_queue(ob_first,s16_To_Int(bufX16),loc,1,'X'); // BYTE 的 X佔1 
				}
				// C need to -> hex
				else if(Input_buff[0] == 'C'){ // EX: C'EOF'
					char bufX16[4];
					bufX16[0] = Input_buff[2];
					bufX16[1] = Input_buff[3];
					bufX16[2] = Input_buff[4];
					bufX16[3] = '\0';
					fprintf(source_pass2,"%X\t%s\t%s\t%s\t\t%s%02X%02X%02X\n",loc,symbol_buff,opcode_setting,Input_buff,ob_first,bufX16[0],bufX16[1],bufX16[2]);
					add_queue(bufX16,0,loc,3,'C'); // BYTE 的 C佔3 
				} 
			}
			// WORD need to -> hex
			else if(!stricmp(opcode_setting,"WORD")){  
				fprintf(source_pass2,"%X\t%s\t%s\t%s\t\t%s%06X\n",loc,symbol_buff,opcode_setting,Input_buff,ob_first,s10_To_Int(Input_buff));
				add_queue(ob_first,s10_To_Int(Input_buff),loc,3,'W'); // WORD 的佔3 
			}
			// RESB RESW 沒 object code 
			else if((!stricmp(opcode_setting,"RESB" ) || (!stricmp(opcode_setting,"RESW")))){ 
				fprintf(source_pass2,"%X\t%s\t%s\t%s\n",loc,symbol_buff,opcode_setting,Input_buff);  
			}
		}
		else{
			// 在symbol table有的 
			// copy opcode to ob_first
			// 1000	FIRST	STL	RETADR		141033
			//                              YY   
			strcpy(ob_first,pointer->op_num); 
			objcode = 0;
			ob_second = 0;
			int i;
			for(i = 0;i < top;i++){
				// 在symbol table找source的input data在stack的哪裡 
				if(!stricmp((symbol_table+i)->text, Input_buff)){
					break;
				}
			}
			if(i != top){
				// 1000	FIRST	STL	RETADR		141033
				//                                YYYY  
				ob_second=(symbol_table+i)->address;  
				fprintf(source_pass2,"%X\t%s\t%s\t%s\t\t%s%04X\n",loc,symbol_buff,opcode_setting,Input_buff,ob_first,ob_second);
			}else if(Input_buff[strlen(Input_buff)-2] == ','&&Input_buff[strlen(Input_buff)-1] == 'X'){
				fprintf(source_pass2,"%X\t%s\t%s\t%s\t%s",loc,symbol_buff,opcode_setting,Input_buff,ob_first);
				Input_buff[strlen(Input_buff)-2] = '\0';
				for(i = 0;i < top;i++){
					if(!stricmp((symbol_table+i)->text, Input_buff)){ // 找到input data在symbol table 的位置 
						break;
					}
				}
				// ob_second = 該行input data在symbol紀錄的位址 
				ob_second=(symbol_table+i)->address;
				ob_second += 32768;
				fprintf(source_pass2,"%04X\n",ob_second);
			}else{
				fprintf(source_pass2,"%X\t%s\t%s\t%s\t\t%s%04X\n",loc,symbol_buff,opcode_setting,Input_buff,ob_first,ob_second);
			}
			
			add_queue(ob_first,ob_second,loc,3,'E'); 
		}
		// 不同形式 位移量不同 
		if(!stricmp(opcode_setting,"BYTE")){
			if(Input_buff[0] == 'X'){	
				loc += 1;
			}
			else if(Input_buff[0] == 'C'){
				loc += 3;
			} 
		}
		else if(!stricmp(opcode_setting,"WORD")){
			loc += 3;
		}
		else if(!stricmp(opcode_setting,"RESB")){
			int cnt = s10_To_Int(Input_buff); // 字串轉int才能算數運算 
			loc += cnt;
		}
		else if(!stricmp(opcode_setting,"RESW")){
			int cnt = s10_To_Int(Input_buff); // 字串轉int才能算數運算 
			loc += (3*cnt);
		}
		else{
			// 其餘的都位移量3 
			loc += 3;
		}
		// last line
		do{
			fscanf(source_code,"%[^\n]",str_buff);
			c_buff = fgetc(source_code);
		}while(commentLine(str_buff));
		get_buff(str_buff,symbol_buff,opcode_setting,Input_buff);
	}
	// print END的 
	fprintf(source_pass2,"\t%s\n",str_buff);
	fclose(source_pass2);
	printf("\'pass2_source_program.txt\' 	has been generated!\n\n"); // 這裡生成 pass2 的 source program 是包含 loc 和 object code 的 
}

//pass 2 - object program 生成 
// 原始程式的機器語言碼
// 可以被載入程式(Loader)載入記憶體中等待執行
void do_object_pro(){
	object_pro = fopen("object_program.txt","w");
	// Header
	// object program開頭 head
	fprintf(object_pro,"H%s\t%06X %06X\n",pgname,start_address,Length); 
	// text part 
	int i,j;
	int sum = 0;
	int start = (queue+0)->locat;
	int startindex = 0;
	int len = 0;
	for(i = 0;i < rear;i++){  // i+1 : read a element from queue
		sum = (queue+i)->locat - start; // queue's length
		// 算10個傢伙一排 但有人只占記憶體size=1 
		if(sum >= 28){
			fprintf(object_pro,"T%06X %02X ",start,len);
			for(j = startindex;j<i;j++){  
				// different type
				switch ((queue+j)->flag){
					case 'C' : // BYTE 
						fprintf(object_pro,"%02X%02X%02X ", (queue+j)->ob_first[0], (queue+j)->ob_first[1], (queue+j)->ob_first[2]); // EX: C'EOF'
						break;
					case 'X': // BYTE 
						fprintf(object_pro,"%s%02X ", (queue+j)->ob_first, (queue+j)->data_location); // EX: X'F1' 
						break;
					case 'W': // WORD
						fprintf(object_pro,"%s%06X ", (queue+j)->ob_first, (queue+j)->data_location);
						break;
					case 'E': // symbol table內有的傢伙們 
						fprintf(object_pro,"%s%04X ", (queue+j)->ob_first, (queue+j)->data_location);
						break;
				}
			}
			fprintf(object_pro,"\n");
			// 起始點賦予新位址 
			start = (queue+i)->locat;
			startindex = i; // 紀錄剛跑到第幾個 
			len = 0;
		}
		len += (queue+i)->add_queue;
	}
	// T part的最後 
	fprintf(object_pro,"T%06X %02X ",start,len);
	for(j = startindex; j<i; j++){
		switch ((queue+j)->flag){
			// 依照不同flag print不同data 
			case 'C' : // BYTE - X
				fprintf(object_pro,"%02X%02X%02X ",(queue+j)->ob_first[0],(queue+j)->ob_first[1],(queue+j)->ob_first[2]);
				break;
			case 'B': // BYTE - C
				fprintf(object_pro,"%s%02X ",(queue+j)->ob_first,(queue+j)->data_location);
				break;
			case 'W': // WORD
				fprintf(object_pro,"%s%06X ",(queue+j)->ob_first,(queue+j)->data_location);
				break;
			case 'E': // symbol table內有的傢伙們 
				fprintf(object_pro,"%s%04X ",(queue+j)->ob_first,(queue+j)->data_location);
			break;
		}
	}
	fprintf(object_pro,"\n"); 
	// end
	fprintf(object_pro,"E%06X\n",start_address); 
	fclose(object_pro); // 有始有終 
	printf("\'object_program.txt\'		has been generated!"); // 這是最後生成的 object program 
} 

// 檢查index用 建立op_table時按照字典序
// index=0 :|ADD  |ADDF |ADDR |...|
// index=2 :|CLEAR|COMP |COMPF|...|
// ...
void check_index(int index, char *string_buff){
			
		if(*(op_table + index) == NULL){ 
			// 沒有出現過,沒存檔紀錄
			// 建立一個memory給他 
			*(op_table + index) = (opcodeStruct*)malloc(sizeof(opcodeStruct));
			opcodeStruct* pointer = *(op_table + index);
			//opcode那行 + 對應的pointer填入op_table 
			opcode_setting(pointer,string_buff); 
		}
		else{  
			// 這邊是出現過的傢伙們
			// 存入過的 ,index有過紀錄
			opcodeStruct* pointer = *(op_table + index);
			//找尾端 接上malloc的new memory (EX: add_queue 18 後面新的一塊momery要接上 ADDF 58) 
			while(pointer->next != NULL){ 
				pointer = pointer->next;
			}
			pointer->next =  (opcodeStruct*)malloc(sizeof(opcodeStruct));
			pointer = pointer->next;
			//opcode + 對應的cnt填入op_table 
			opcode_setting(pointer,string_buff); 
		}
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// **main 在這**																											//
//開檔 ->讀opcode ->建立op_table ->calculate location ->print pass1 source(add location) ->generate symbol table and print  //
// ->再開source ->讀statement ->計算object code ->print pass1 source(add object code) ->generate object program and print	//																 																											//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(){
	
	char sourcefile[20] = "sic.txt";
	char opCodefile[20] = "opcode.txt";
	
	if((opCode = fopen(opCodefile,"r")) == NULL){
		printf("error with open opcode file\n",opCodefile);
		exit(0);
	}
	
	op_table = (opcodeStruct**)malloc(sizeof(opcodeStruct*)*26);
	int i;
	// malloc size of alph = 26 (做開頭用:opcode乃是依照開頭ABC...下去排)
	for(i = 0;i < 26;i++){
		*(op_table+i) =  NULL;
	}
	char c_buff = NULL;
	char string_buff[15];  
	// 建立command_table by op_table(opcode.txt) 
	while(c_buff != EOF){  // EOF檔尾 
		fscanf(opCode,"%[^\n]",string_buff); // 把一行讀到的東東先存到sbuff (包括space) 
		int index = string_buff[0]-'A'; // statement都大寫 減掉 A 算出opcode 的 index (A = 0, B = 1...以此類推)
		// 檢查index 
		check_index(index,string_buff);
		// 多一個'\n' 讀起來放 防止'\n'跑到下一行資料內
		c_buff = fgetc(opCode); // opCode('opcode.txt')任務結束 
	}
	fclose(opCode);
	
	// open sourcefile: pass1
	if((source_code = fopen(sourcefile,"r")) == NULL){
		printf("error with open source file\n");
		exit(0);
	}
	// pass1 啟動
	// 2個東西生成 -> pass1_source_program, symbol_table
	do_pass1_source();
	do_SymbolTable();
	fclose(source_code);
	
	// open sourcefile: pass2
	if((source_code = fopen(sourcefile,"r")) == NULL){
		printf("error with open source file\n");
		exit(0);
	}
	// pass2 啟動
	// 2個東西生成 -> pass2_source_program, object_program
	do_pass2_source();
	do_object_pro();
	fclose(source_code);
   	
	return 0;
}
