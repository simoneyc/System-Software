# include <stdio.h>
# include <stdlib.h>
# include <string.h>
// file variable
FILE *source_pass1,*source_pass2,*object_pro,*symbol_TAB,*source_code,*opCode;
int Length,top = 0,symTab_size = 2, loc, start_address=0;
// define op_table from 'opcode'
// �x�sinput opcode �� data 
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
// ���o��: 
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
// ��opcode�s�n 
void opcode_setting(opcodeStruct* pointer, char* readline){ 
	pointer->next = NULL;
	// �Hspace���j: 'add_queue 18'
	int i;
	for(i = 0;readline[i] != ' ';i++){
		pointer->text[i] = readline[i];
	}
	// Ū'\0' 16�i��n�ഫ 
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
	// ���L space or tab
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
	// ���L space or tab
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
//**pass1 �}�l**																													//
//pass1 ���Ȧ�: 																													//
//Calculate the loc value 																											//
//Create the symbol table 																											//
//Generate the immediate file																										//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// pass1

// stack-push function
// �إ�stack�ΥH�Ы�symbol table 
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

// ���X��loc value�� source program
// �o��ͦ� pass1's source_program
void do_pass1_source(){
	source_pass1 = fopen("pass1_source_program.txt","w");
	char str_buff[200]; // symbol opcode
	char c_buff;
	char symbol_buff[50];
	char opcode_setting[50];
	char Input_buff[50];
	// ���tmemory��symbol table 
	symbol_table = (symbolStruct*)malloc(symTab_size*sizeof(symbolStruct));
	// Ū, if �J�� comment �h���U
	do{
		fscanf(source_code,"%[^\n]",str_buff);
		c_buff = fgetc(source_code); // �Ȧs"\n"   
	}while(commentLine(str_buff)); // ���L���� 
	
	// ��ӧOdata 
	get_buff(str_buff,symbol_buff,opcode_setting,Input_buff);
	int i;
	
	// �� START ���᪺ line 
	if(!stricmp(opcode_setting,"START")){
		start_address = s16_To_Int(Input_buff);
		loc = start_address;
		// �v�� print �X 
		fprintf(source_pass1,"%X\t%s\n",loc,str_buff);
	}
	else{
		// START ���� LOC �] 0 
		loc = 0;
	}
	
	// �P�e�z����,���L comment  
	do{
		fscanf(source_code,"%[^\n]",str_buff); 
		c_buff = fgetc(source_code);
	}while(commentLine(str_buff));
	get_buff(str_buff,symbol_buff,opcode_setting,Input_buff);
	   
	// END ���פ�T�� 
	while(stricmp(opcode_setting,"END")){ // END ���ܭ�l�{���������B �ë��w�{�����Ĥ@�ӥi���檺���O
		// print
		fprintf(source_pass1,"%X\t%s\n",loc,str_buff);
		
		// push �h�ͥXsymbolTAB 
		if(symbol_buff[0] != '\0'){
			push(symbol_buff);
		}
		// �p��U�@�檺 loc (��opcode) 
		// BYTE,WORD�O�ഫ�줸or�r�ի��O,RESB,RESW�O�d�O����Ŷ�
		if(!stricmp(opcode_setting,"BYTE")){ // �w�q�r���ΤQ���i�쪺�`�� �åB���X��i���Τ��줸�ժ��ƶq
			if(Input_buff[0] == 'X'){ // �Q���i��ƭ� ASCII 
				loc += 1;
			}
			else if(Input_buff[0] == 'C'){ // �r�� +3���  
				loc += 3;
			} 
		}
		else if(!stricmp(opcode_setting,"WORD")){ // �w�q�@�Ӧr�ժ���Ʊ`��
			loc += 3;
		}
		else if(!stricmp(opcode_setting,"RESB")){ // RESB �O�d�ҥܼƶq���줸�� �Ѹ�ưϨϥ�
			int cnt = s10_To_Int(Input_buff);
			loc += cnt;
		}
		else if(!stricmp(opcode_setting,"RESW")){ // RESW �O�d�ҥܼƶq���r�� �Ѹ�ưϨϥ�
			int cnt = s10_To_Int(Input_buff);
			loc += (3*cnt); // RESW �O WORD
		}
		// �d�� 
		else{
			opcodeStruct* pointer = check_opcodeTab(opcode_setting);
			if(pointer == NULL){
				// ���� 
				printf("error with opcode\n");
				exit(0);
			}
			loc += 3;
		}
		
		// Ū�U�@�� 
		do{
			fscanf(source_code,"%[^\n]",str_buff);
			c_buff = fgetc(source_code);
		}while(commentLine(str_buff));
		get_buff(str_buff,symbol_buff,opcode_setting,Input_buff);
	}
	
	//	print END line
	fprintf(source_pass1,"\t%s\n",str_buff);
	Length = loc-start_address; // �{���`���� END - START 
	fclose(source_pass1);
	printf("\'pass1_source_program.txt\' 	has been generated!\n\n"); // pass1_source_program.txt �O pass1 ���]�t loc �� txt ���٨S�]�t object code 
}

// generate - SymbolTable �g�J file 
void do_SymbolTable(){
	symbol_TAB = fopen("symbol_table.txt","w");
	int i;
	for(i = 0;i < top;i++){
		fprintf(symbol_TAB,"%s\t%X\n",(symbol_table+i)->text,(symbol_table+i)->address); // symbol_table�O����B+i�ӴM��symbol table�� data 
	}
	fclose(symbol_TAB);
	printf("\'symbol_table.txt\' 		has been generated!\n\n"); // �o�̥ͦ�symbol table 
}

// check if 'comment line'
int commentLine(char* Line){
	int i;
	for(i = 0;i < strlen(Line);i++){
		if(Line[i] != ' '&&Line[i] != '\t')	
			break;
	}
	// comment �O . 
	if(Line[i] == 46){
		return 1;
	}
	else{
		return 0;
	}
}

// (Hex in str) to Int : s16 -> 10
int s16_To_Int(char* Input_buff){ // ��J�Ѽ�'Input_buff'���V�r�żƲ�
	int ten = 0; // ten = 0 �Ω�s�̫᪺�Q�i��
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
	int ten = 0; // ten = 0 �Ω�s�̫᪺�Q�i��
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

// find opCode in op_table �d�� 
// return LOC
// �d opcode's function 
opcodeStruct* check_opcodeTab(char* opcode_setting){ // �Ѽ�opcode_setting��ܭn�j����opcode�O�� 
	opcodeStruct* pointer;
	if(opcode_setting[0] >= 'A'&&opcode_setting[0] <= 'Z'){
		pointer = *(op_table + opcode_setting[0]-'A');// �p��Xindex�A�Ө�ƴN�|�˯����VopCodeUnit��index�BLink���Ĥ@��pointer
		while(pointer != NULL){
			if(!stricmp(pointer->text,opcode_setting)){  // ���F! break �h return ���� pointer���V���å� if (s1 == s2) , return 0
				break;
			}
			pointer = pointer->next; // ���U��
		}
	}
	else{
		printf("error with opcode\n"); // print if error exist
		exit(0);
	}
	return pointer; // return pointer
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//**pass2 �}�l**																													//
//pass2 ���Ȧ�: 																													//
//�z�L�p��object code�o��̫᪺object program 																						//
//�ҥH�]��[�Jobject code�᪺source fprintf�X�� 																					//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// �إ�queue 
// �ͦ�object program�� 
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

// pass 2 -  SourceProgram �ͦ�
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
	// object_code ���e�� �e��first=symbol�d�쪺�Ʀr second=(symbol_table+index)��address 
	 
	// ���B�z�Ĥ@�� COPY	START	1000
	do{
		fscanf(source_code,"%[^\n]",str_buff);
		c_buff = fgetc(source_code);
	}while(commentLine(str_buff)); // ignore comments 
	
	get_buff(str_buff,symbol_buff,opcode_setting,Input_buff);
	strcpy(pgname,symbol_buff); // program name = first symbol_buff ��쪺�å� 
	// first line
	loc = start_address;
	fprintf(source_pass2,"%X\t%s\n",loc,str_buff);
	// �B�z�Ĥ@��H�᪺�å�� 
	do{
		fscanf(source_code,"%[^\n]",str_buff);
		c_buff = fgetc(source_code);
	}while(commentLine(str_buff));// ignore comments 
	
	get_buff(str_buff,symbol_buff,opcode_setting,Input_buff);
	// malloc memory �� queue 
	queue = (link*)malloc(sizeof(link)*1000);
	
	while(stricmp(opcode_setting,"END")){
		pointer = check_opcodeTab(opcode_setting); // find opcode
		if (pointer == NULL){ // ��Ѫ��� NULL (�S��) ���bsymbol table�̭��� 
			strcpy(ob_first,"");  // �ƻs�Ū��������X�� ob_first 
			if(!stricmp(opcode_setting,"BYTE")){  //stricmp �۵� : = 0
				// BYTE ����� 
				if(Input_buff[0] == 'X'){ // EX: X'F1'
					char bufX16[3];
					bufX16[0] = Input_buff[2];
					bufX16[1] = Input_buff[3];
					bufX16[2] = '\0';
					fprintf(source_pass2,"%x\t%s\t%s\t%s\t\t%s%02X\n",loc,symbol_buff,opcode_setting,Input_buff,ob_first,s16_To_Int(bufX16));
					// into queue
					add_queue(ob_first,s16_To_Int(bufX16),loc,1,'X'); // BYTE �� X��1 
				}
				// C need to -> hex
				else if(Input_buff[0] == 'C'){ // EX: C'EOF'
					char bufX16[4];
					bufX16[0] = Input_buff[2];
					bufX16[1] = Input_buff[3];
					bufX16[2] = Input_buff[4];
					bufX16[3] = '\0';
					fprintf(source_pass2,"%X\t%s\t%s\t%s\t\t%s%02X%02X%02X\n",loc,symbol_buff,opcode_setting,Input_buff,ob_first,bufX16[0],bufX16[1],bufX16[2]);
					add_queue(bufX16,0,loc,3,'C'); // BYTE �� C��3 
				} 
			}
			// WORD need to -> hex
			else if(!stricmp(opcode_setting,"WORD")){  
				fprintf(source_pass2,"%X\t%s\t%s\t%s\t\t%s%06X\n",loc,symbol_buff,opcode_setting,Input_buff,ob_first,s10_To_Int(Input_buff));
				add_queue(ob_first,s10_To_Int(Input_buff),loc,3,'W'); // WORD ����3 
			}
			// RESB RESW �S object code 
			else if((!stricmp(opcode_setting,"RESB" ) || (!stricmp(opcode_setting,"RESW")))){ 
				fprintf(source_pass2,"%X\t%s\t%s\t%s\n",loc,symbol_buff,opcode_setting,Input_buff);  
			}
		}
		else{
			// �bsymbol table���� 
			// copy opcode to ob_first
			// 1000	FIRST	STL	RETADR		141033
			//                              YY   
			strcpy(ob_first,pointer->op_num); 
			objcode = 0;
			ob_second = 0;
			int i;
			for(i = 0;i < top;i++){
				// �bsymbol table��source��input data�bstack������ 
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
					if(!stricmp((symbol_table+i)->text, Input_buff)){ // ���input data�bsymbol table ����m 
						break;
					}
				}
				// ob_second = �Ӧ�input data�bsymbol��������} 
				ob_second=(symbol_table+i)->address;
				ob_second += 32768;
				fprintf(source_pass2,"%04X\n",ob_second);
			}else{
				fprintf(source_pass2,"%X\t%s\t%s\t%s\t\t%s%04X\n",loc,symbol_buff,opcode_setting,Input_buff,ob_first,ob_second);
			}
			
			add_queue(ob_first,ob_second,loc,3,'E'); 
		}
		// ���P�Φ� �첾�q���P 
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
			int cnt = s10_To_Int(Input_buff); // �r����int�~���ƹB�� 
			loc += cnt;
		}
		else if(!stricmp(opcode_setting,"RESW")){
			int cnt = s10_To_Int(Input_buff); // �r����int�~���ƹB�� 
			loc += (3*cnt);
		}
		else{
			// ��l�����첾�q3 
			loc += 3;
		}
		// last line
		do{
			fscanf(source_code,"%[^\n]",str_buff);
			c_buff = fgetc(source_code);
		}while(commentLine(str_buff));
		get_buff(str_buff,symbol_buff,opcode_setting,Input_buff);
	}
	// print END�� 
	fprintf(source_pass2,"\t%s\n",str_buff);
	fclose(source_pass2);
	printf("\'pass2_source_program.txt\' 	has been generated!\n\n"); // �o�̥ͦ� pass2 �� source program �O�]�t loc �M object code �� 
}

//pass 2 - object program �ͦ� 
// ��l�{���������y���X
// �i�H�Q���J�{��(Loader)���J�O���餤���ݰ���
void do_object_pro(){
	object_pro = fopen("object_program.txt","w");
	// Header
	// object program�}�Y head
	fprintf(object_pro,"H%s\t%06X %06X\n",pgname,start_address,Length); 
	// text part 
	int i,j;
	int sum = 0;
	int start = (queue+0)->locat;
	int startindex = 0;
	int len = 0;
	for(i = 0;i < rear;i++){  // i+1 : read a element from queue
		sum = (queue+i)->locat - start; // queue's length
		// ��10�ӳå�@�� �����H�u�e�O����size=1 
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
					case 'E': // symbol table�������å�� 
						fprintf(object_pro,"%s%04X ", (queue+j)->ob_first, (queue+j)->data_location);
						break;
				}
			}
			fprintf(object_pro,"\n");
			// �_�l�I�ᤩ�s��} 
			start = (queue+i)->locat;
			startindex = i; // ������]��ĴX�� 
			len = 0;
		}
		len += (queue+i)->add_queue;
	}
	// T part���̫� 
	fprintf(object_pro,"T%06X %02X ",start,len);
	for(j = startindex; j<i; j++){
		switch ((queue+j)->flag){
			// �̷Ӥ��Pflag print���Pdata 
			case 'C' : // BYTE - X
				fprintf(object_pro,"%02X%02X%02X ",(queue+j)->ob_first[0],(queue+j)->ob_first[1],(queue+j)->ob_first[2]);
				break;
			case 'B': // BYTE - C
				fprintf(object_pro,"%s%02X ",(queue+j)->ob_first,(queue+j)->data_location);
				break;
			case 'W': // WORD
				fprintf(object_pro,"%s%06X ",(queue+j)->ob_first,(queue+j)->data_location);
				break;
			case 'E': // symbol table�������å�� 
				fprintf(object_pro,"%s%04X ",(queue+j)->ob_first,(queue+j)->data_location);
			break;
		}
	}
	fprintf(object_pro,"\n"); 
	// end
	fprintf(object_pro,"E%06X\n",start_address); 
	fclose(object_pro); // ���l���� 
	printf("\'object_program.txt\'		has been generated!"); // �o�O�̫�ͦ��� object program 
} 

// �ˬdindex�� �إ�op_table�ɫ��Ӧr���
// index=0 :|ADD  |ADDF |ADDR |...|
// index=2 :|CLEAR|COMP |COMPF|...|
// ...
void check_index(int index, char *string_buff){
			
		if(*(op_table + index) == NULL){ 
			// �S���X�{�L,�S�s�ɬ���
			// �إߤ@��memory���L 
			*(op_table + index) = (opcodeStruct*)malloc(sizeof(opcodeStruct));
			opcodeStruct* pointer = *(op_table + index);
			//opcode���� + ������pointer��Jop_table 
			opcode_setting(pointer,string_buff); 
		}
		else{  
			// �o��O�X�{�L���å��
			// �s�J�L�� ,index���L����
			opcodeStruct* pointer = *(op_table + index);
			//����� ���Wmalloc��new memory (EX: add_queue 18 �᭱�s���@��momery�n���W ADDF 58) 
			while(pointer->next != NULL){ 
				pointer = pointer->next;
			}
			pointer->next =  (opcodeStruct*)malloc(sizeof(opcodeStruct));
			pointer = pointer->next;
			//opcode + ������cnt��Jop_table 
			opcode_setting(pointer,string_buff); 
		}
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// **main �b�o**																											//
//�}�� ->Ūopcode ->�إ�op_table ->calculate location ->print pass1 source(add location) ->generate symbol table and print  //
// ->�A�}source ->Ūstatement ->�p��object code ->print pass1 source(add object code) ->generate object program and print	//																 																											//
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
	// malloc size of alph = 26 (���}�Y��:opcode�D�O�̷Ӷ}�YABC...�U�h��)
	for(i = 0;i < 26;i++){
		*(op_table+i) =  NULL;
	}
	char c_buff = NULL;
	char string_buff[15];  
	// �إ�command_table by op_table(opcode.txt) 
	while(c_buff != EOF){  // EOF�ɧ� 
		fscanf(opCode,"%[^\n]",string_buff); // ��@��Ū�쪺�F�F���s��sbuff (�]�Aspace) 
		int index = string_buff[0]-'A'; // statement���j�g � A ��Xopcode �� index (A = 0, B = 1...�H������)
		// �ˬdindex 
		check_index(index,string_buff);
		// �h�@��'\n' Ū�_�ө� ����'\n'�]��U�@���Ƥ�
		c_buff = fgetc(opCode); // opCode('opcode.txt')���ȵ��� 
	}
	fclose(opCode);
	
	// open sourcefile: pass1
	if((source_code = fopen(sourcefile,"r")) == NULL){
		printf("error with open source file\n");
		exit(0);
	}
	// pass1 �Ұ�
	// 2�ӪF��ͦ� -> pass1_source_program, symbol_table
	do_pass1_source();
	do_SymbolTable();
	fclose(source_code);
	
	// open sourcefile: pass2
	if((source_code = fopen(sourcefile,"r")) == NULL){
		printf("error with open source file\n");
		exit(0);
	}
	// pass2 �Ұ�
	// 2�ӪF��ͦ� -> pass2_source_program, object_program
	do_pass2_source();
	do_object_pro();
	fclose(source_code);
   	
	return 0;
}
