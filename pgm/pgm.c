//19011067 Ömer Sinan Bilget  
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>
#include<dirent.h>
#define BUFFER 150
#define IS_ACTIVE true

typedef struct pgm{
	int row;
	int col;
	int **image;
	int *compressed;
	int maxval;
	int compressed_size;
}PGM;


PGM* pgm_to_matrix(char*);
bool matrix_to_pgm(PGM *,char *,int);
bool isNumber(char);
bool compress_matrix(PGM*,char*,int);
bool compress_control(char*);
PGM* decompress(char*);
PGM* compress_to_array(char*);
bool array_to_compress(PGM*,char*,int);
bool change_pixel(int*,int,int);
void shift_right(int*,int,int);
void shift_left(int*,int,int,int);
void fill(int*,int*,int,int);
int convert_to_int(char*,int,int);
bool skip(char *);
int** allocate_matrix(int,int);
void free_matrix(int **,int);
bool file_control(char*,char*,int);
bool change_color(PGM*,char*,int,int,int);
void menu();
void press_a_key();
void name_file(char*,char*);
void clear();
char**list_of_files(int*,char*);
void free_list(char**,int);
void print_list(char**,int);
void print_matrix(int**,int,int,int,bool);
void print_array(int*,int,int,bool);
void histogram(int*,char*,int,int);




int main(){
	menu();
	return 0;
}

//is char is number return true else return false
bool isNumber(char c){
	int i;
	if( c >= '0' && c <= '9' ){
		return true;
	}else{
		return false;
	}
}

//convert string to the integer between start and end index
int convert_to_int(char* c,int start,int end){
	char*tmpstr=(char*)malloc((end-start+1)*sizeof(char));
	int i=0;
	for(i=start;i<end;i++){
		tmpstr[i-start]=c[i];
	}
	tmpstr[(end-start)]='\0';
	int num=atoi(tmpstr);
	free(tmpstr);
	return num;
}

//read pgm file and convert to a matrix
PGM* pgm_to_matrix(char* filename){
	PGM *pgm=(PGM*)malloc(sizeof(PGM));
	if(pgm==NULL){
		printf("error : memory allocation");
		return NULL;
	}
	FILE* fp=fopen(filename,"r");
	int row;
	int col;
	int maxval;
	int index=0;
	int i;
	if(fp==NULL){
		printf("error : memory allocation\n");
		fclose(fp);
		return NULL;
	}
	char line[BUFFER];
	char format[2];
	fscanf(fp,"%s\n",&format);
	if(strcmp(format,"P2")!=0){
		printf("error : file type is invalid\n");
		fclose(fp);
		return NULL;
	}
	//0 read row col  1 read  matrix size 
	int read=0;
	while(read<2){
		fgets(line,BUFFER,fp);
		if(skip(line)==false){
			if(read==0){
				sscanf(line,"%d %d\n",&col,&row);
				read=1;
			}else if(read==1){
		
				sscanf(line,"%d\n",&maxval);
				read=2;
			}
		}
	}
	//allocate matrix
	int **m;
	m=allocate_matrix(row,col);
	int start;
	int end;
	bool is_start=false;
	
	//fill matrix
	while(fgets(line,BUFFER,fp)!=NULL){
		for(i=0;i<strlen(line);i++){
			if(isNumber(line[i])==true){
				if(is_start==false){
					is_start=true;
					start=i;
				}
			}else{
				if(is_start==true){
					is_start=false;
					end=i;
					int val=convert_to_int(line,start,end);
					m[index/col][index%col]=val;
					index+=1;
				}
			}
		}
	}
	pgm->image=m;
	pgm->row=row;
	pgm->col=col;
	pgm->maxval=maxval;
	fclose(fp);
	printf("file converted to matrix\n");
	return pgm;
}


int** allocate_matrix(int row,int col){
	int **m;
	m=(int**)malloc(row*sizeof(int*));
	if(m==NULL){
		printf("error : memory allocation error\n");
		return ;
	}
	
	int i;
	for(i=0;i<row;i++){
		m[i]=(int*)malloc(col*sizeof(int));
		if(m[i]==NULL){
			printf("error : memory allocation error\n");
			return;
		}
	}

	return m;
}

void free_matrix(int ** m,int row){
	int i;

	for(i=0;i<row;i++){
		free(m[i]);
	}
	free(m);
}

//skip the comment line in pgm files
bool skip(char *c){
	int i;
	for(i=0;i<strlen(c);i++){
		if(c[i]=='#'){
			return true;
		}
	}
	return false;
}

//convert matrix to a pgm file
bool matrix_to_pgm(PGM *pgm,char *filename,int rowlen){
	FILE* fp=fopen(filename,"w");
	int index=0;
	int i;
	int col=pgm->col;
	if(fp==NULL){
		printf("error\n");
		return false;
	}
	fprintf(fp,"P2\n");
	fprintf(fp,"# %s\n",filename);
	fprintf(fp,"%d %d\n",pgm->col,pgm->row);
	fprintf(fp,"%d\n",pgm->maxval);
	
	for(i=0;i<pgm->row*pgm->col;i++){
		fprintf(fp,"%d\t",pgm->image[i/col][i%col]);
		index+=1;
		if(index>=rowlen){
			index=0;
			fputs("\n",fp);
		}
	}
	free_matrix(pgm->image,pgm->row);
	return true;
}

//encode matrix
bool compress_matrix(PGM* pgm,char* filename,int rowlen){
	FILE *fp=fopen(filename,"w");
	if(fp==NULL){
		printf("error : memory allocation error\n");
		return false;
	}
	int val;
	int tmpval;
	int count=1;
	int rowcount=0;
	int i;
	int col=pgm->col;
	fprintf(fp,"%d %d %d ",pgm->row,pgm->col,pgm->maxval);
	val=pgm->image[0][0];
	for(i=1;i<pgm->row*pgm->col;i++){
		tmpval=pgm->image[i/col][i%col];
		if(tmpval==val){
			count+=1;
		}else{
			fprintf(fp,"%d %d ",count,val);
			rowcount+=1;
			if(rowcount>=rowlen){
				fputs("\n",fp);
				rowcount=0;
			}
			val=tmpval;
			count=1;
		}
	}
	fprintf(fp,"%d %d ",count,val);
	free_matrix(pgm->image,pgm->row);
	free(pgm);
	fclose(fp);
	printf("matrix converted to compressed file\n");
	return true;
}



void menu(){
	char filename[BUFFER];
	bool end=false;
	bool end2=false;
	bool end3=false;
	bool end4=false;
	char option;
	char option2;
	char option3;
	char option4;
	char option5;
	int check=0;
	int x,y;
	int color;
	int newcolor;
	int location;
	int listsize;
	int listindex;
	char compressed[]="compressed.txt";
	while(end==false){
		printf("------------------------------------------------\n");
		printf("     19011067   Omer   Sinan   Bilget           \n");
		printf("1.Compress PGM File                             \n");
		printf("2.Open Compressed File                          \n");
		printf("3.Exit                                          \n");
		printf("------------------------------------------------\n");
		option=getch();
		if(option>'0' && option <='3'){
			switch(option){
				case '1':
					end2=false;
					while(end2==false){
						clear();
						printf("1.enter file name\n");
						printf("2.return\n");
						option2=getch();
						switch(option2){
							case '1':
								printf("\n");
								printf("1.enter filename(pgm file)\n");
								printf("2.choose file from current directory\n");
								printf("\n");
								option5=getch();
								if(option5=='1'){
									printf("filename :");
									scanf("%s",filename);
								}else if(option5=='2'){
									listsize=0;
									char**list=list_of_files(&listsize,"pgm");
									print_list(list,listsize);
									printf("enter number: ");
									scanf("%d",&listindex);
									if(listindex>=1 && listindex<=listsize){
										filename[0]='\n';
								    	strcpy(filename,list[listindex-1]);
								    	printf("%s\n",filename);
								    	free_list(list,listsize);
									}else{
										printf("error : invalid value\n");
									}
								}else{
									printf("error : invalid value");
								}
								printf("\n");
				            	if(file_control(filename,"pgm",1)==true){
				            		check=0;
									printf("Converting to compressed txt file...\n");
									PGM *pgm;
									pgm=pgm_to_matrix(filename);
									if(pgm!=NULL){
										check+=1;
									}
									name_file(filename,"_encoded.txt");
									if(compress_matrix(pgm,filename,30)==true){
										check+=1;
									}
									if(check==2){
										printf("process completed\n");
									}
									end2=true;
									press_a_key();
								}else{
									printf("please enter filename again\n");
									press_a_key();
								}
								break;
							case '2':
								end2=true;
								break;
						}
					}
					break;
				case '2':
					end2=false;
					while(end2==false){
						clear();
						printf("1.enter file name\n");
						printf("2.return\n");
						option2=getch();
						switch(option2){
							case '1':
								printf("\n");
								printf("1.enter filename(txt file)\n");
								printf("2.choose file from current directory\n");
								printf("\n");
								option5=getch();
								if(option5=='1'){
									printf("filename : ");
									scanf("%s",filename);
								}else if(option5=='2'){
									listsize=0;
									char**list=list_of_files(&listsize,"txt");
									print_list(list,listsize);
									printf("enter number: ");
									scanf("%d",&listindex);
									if(listindex>=1 && listindex<=listsize){
										filename[0]='\n';
								    	strcpy(filename,list[listindex-1]);
								    	printf("%s\n",filename);
								    	free_list(list,listsize);
									}else{
										printf("error : invalid value");
									}
								}else{
									printf("error : invalid value");
								}
								printf("\n");
				            	if(file_control(filename,"txt",1)==true){
				            		check=0;
				            		if(compress_control(filename)==true){
				            			printf("control completed\n");
				            			press_a_key();
				            			end3=false;
				            			while(end3==false){
				            				clear();
				            				printf("1.decompress to PGM file\n");
				            				printf("2.edit file\n");
				            				printf("3.print graphic\n");
				            				printf("4.return\n");
				            				option3=getch();
				            				switch(option3){
				            					case '1':{
				            						PGM* p=decompress(filename);
				            						name_file(filename,"_decoded.pgm");
				            						if(matrix_to_pgm(p,filename,30)==true){
				            							printf("process complete\n");
													}else{
														printf("an error occured\n");
													}
													press_a_key();
													end3=true;
													break;
												}
				            						
				            					case '2':{
				            						end4=false;
				            						while(end4==false){
				            							clear();
				            							printf("1.Add pixel to given coordinates\n");
				            							printf("2.change a color to another color\n");
				            							printf("3.return\n");
				            							option4=getch();
				            							check=0;
				            							switch(option4){
				            								case '1':{
				            									PGM* pgm=compress_to_array(filename);
				            									if(pgm==NULL){
				            										printf("memory allocation error\n");
				            										return;
																}
																printf("\n");
																printf("enter coordinates\n");
																printf("x(0-%d):",pgm->col-1);
																scanf("%d",&x);
																printf("y(0-%d):",pgm->row-1);
																scanf("%d",&y);
																printf("enter color(0-%d) :",pgm->maxval);
																scanf("%d",&color);
																if(x>=0 && x<pgm->col && y>=0 && y<pgm->row){
																	printf("valid coordinates\n");
																	check+=1;
																}else{
																	printf("error : invalid coordinates\n");
																}
																if(color>=0 && color<=pgm->maxval){
																	printf("color is valid\n");
																	check+=1;
																}else{
																	printf("error : color is invalid\n");
																}
																if(check==2){
																	printf("updating the array...\n");
																	location=(pgm->col*y)+x;
																	printf("%d\n",location);
																	if(change_pixel(pgm->compressed,location+1,color)==true){
																		printf("updating the file...\n");
																		
																		print_array(pgm->compressed,pgm->compressed_size,20,IS_ACTIVE);
																		if(array_to_compress(pgm,filename,30)==true){
																			printf("file updated\n");
																		}
																	    end4=true;
																	    end3=true;
																	}
																	
																}
																press_a_key();
	
																break;
															}	
				            								case '2':{
				            									PGM* pgm=compress_to_array(filename);
				            									if(pgm==NULL){
				            										printf("memory allocation error\n");
				            										return;
																}
																printf("\n");
																printf("enter color(0-%d):",pgm->maxval);
																scanf("%d",&color);
																printf("enter new color(0-%d):",pgm->maxval);
																scanf("%d",&newcolor);
																if(color!=newcolor){
																	
																	if(change_color(pgm,filename,color,newcolor,15)==true){
																		printf("color changed\n");
																		end4=true;
																	    end3=true;
																	}
																	
																}else{
																	printf("error : colors are same\n");
																}
																press_a_key();
				            									break;
															}
				            								
				            								case '3':
				            									end4=true;
				            									break;
														}
													}
													break;
												}	
				            					case '3':{
				            						PGM* pgm=compress_to_array(filename);
				            						name_file(filename,"_histogram.txt");
				            						histogram(pgm->compressed,filename,pgm->maxval,pgm->compressed_size);
				            						press_a_key();
				            						end3=true;
				            						break;
												}
				            					case '4' :
				            							end3=true;
				            						break;
											}
										}
									}else{
										press_a_key();
									}
								}else{
									printf("please enter filename again\n");
									press_a_key();
								}
								break;
							case '2':
								end2=true;
								break;
						}
					}
					break;
				case '3':
					end=true;
					break;
			}
		}else{
			printf("invalid input\n");
			press_a_key();
		}	
		clear();
		
	}
}


//if file is valid return true else return false
bool file_control(char* filename,char* filetype,int mode){
	FILE *fp=fopen(filename,"r");
	if(fp==NULL){
		if(mode==1){
			printf("error : file opening failed\n");
		}
		return false;
	}else{
		if(mode==1){
			printf("file exists\n");
		}
		fclose(fp);
	}
	int start;
	int end;
	int dot;
	int i;
	for(i=0;i<strlen(filename);i++){
		if(filename[i]=='.'){
			dot=i;
		}
	}
	for(i=dot+1;i<dot+1+strlen(filetype);i++){
		if(filename[i]!=filetype[i-(dot+1)]){
			if(mode==1){
			   	printf("error: invalid filetype\n");
	    	}
			fclose(fp);
			return false;
		}
	}
	if(strlen(filename)==dot+1+strlen(filetype)){
		if(mode==1){
			printf("valid file type\n");
	    }
		fclose(fp);
    	return true;
	}else{
		if(mode==1){
			printf("error: invalid filetype\n");
	    }
		fclose(fp);
		return false;
	}
}

void press_a_key(){
	printf("press a key to continue\n");
	getch();
}


void clear(){
	#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
	system("cls");
	#endif
}

//rename file
void name_file(char* filename,char* filename2){
	bool dotfound;
	int dot;
	int i=0;
	for(i=0;i<strlen(filename);i++){
		if(dotfound==false){
			if(filename[i]=='.'){
				dotfound=true;
				filename[i]='\0';
				dot=i;
			}
		}else{
			filename[i]='\0';
		}
	}
	strcat(filename,filename2);
}

//check if compressed file is valid return true else return false
bool compress_control(char* filename){
	FILE* fp=fopen(filename,"r");
	if(fp==NULL){
		printf("error : file opening failed\n");
		return false;
	}
	int row;
	int col;
	int count=0;
	bool iscount=true;
	int val;
	int prev_val=-1;
	bool repeated_val=false;
	int maxval;
	int pixel_count=0;
	bool pixel_control=true;
	int max_pixel=255;
	fscanf(fp,"%d %d %d ",&row,&col,&maxval);
	while(fscanf(fp,"%d%*1[\n]",&val)!=EOF){
		count+=1;
		if(iscount==true){
			pixel_count+=val;
		}else{
			if(prev_val==val){
				repeated_val=true;
			}
			prev_val=val;
			if(val>max_pixel){
				pixel_control=false;
			}
		}
		iscount=!iscount;
	}
	if(pixel_count!=row*col){
		printf("%d %d\n",pixel_count,row*col);
		printf("error : pixel count do not match\n");
		fclose(fp);
		return false;
	}else if(count%2==1){
		printf("error : missing value\n");
		fclose(fp);
		return false;
	}else if(pixel_control==false){
		printf("error : some pixels has value greater than %d\n",max_pixel);
		fclose(fp);
		return false;
	}else if(repeated_val==true){
		printf("error : repeated values\n");
		fclose(fp);
		return false;
	}
	printf("compressed file is valid\n");
	fclose(fp);
	return true;
}


//convert compress file to a matrix
PGM* decompress(char *filename){
	FILE* fp=fopen(filename,"r");
	if(fp==NULL){
		printf("error : file opening failed\n");
		return NULL;
	}
	PGM* pgm=(PGM*)malloc(sizeof(PGM));
	if(pgm==NULL){
		printf("error : memory allocation\n");
		return NULL;
	}
	int row;
	int col;
	int maxval;
	int index=0;
	int count=0;
	bool iscount=true;
	int i;
	int val;
	fscanf(fp,"%d %d %d ",&row,&col,&maxval);
	int**m=allocate_matrix(row,col);
	if(m==NULL){
		printf("error : memory allocation\n");
		fclose(fp);
		return NULL;
	}
	while(fscanf(fp,"%d%*1[\n]",&val)!=EOF){
		if(iscount==true){
			count=val;
		}else{
			for(i=index;i<index+count;i++){
				m[i/col][i%col]=val;
			
			}
			index=i;
		}
		iscount=!iscount;
	}
	pgm->row=row;
	pgm->col=col;
	pgm->maxval=maxval;
	pgm->image=m;
	return pgm;
}


//convert compressed file to an array
PGM* compress_to_array(char*filename){
	FILE* fp=fopen(filename,"r");
	if(fp==NULL){
		printf("error : file opening failed\n");
		return NULL;
	}
	PGM* pgm=(PGM*)malloc(sizeof(PGM));
	if(pgm==NULL){
		printf("error : memory allocation\n");
		fclose(fp);
		return NULL;
	}
	int row;
	int col;
	int maxval;
	int index=0;
	bool iscount=true;
	int i;
	int val;
	int arraysize=0;
	long int position;
	fscanf(fp,"%d %d %d ",&row,&col,&maxval);
	while(fscanf(fp,"%d%*1[\n]",&val)!=EOF){
		arraysize+=1;
	}
	//extra 5 size is for change pixel function 
	int *a=(int*)malloc((arraysize+5)*sizeof(int));
	if(a==NULL){
		printf("error : memory allocation\n");
		fclose(fp);
		return NULL;
	}
	fseek(fp,0,SEEK_SET);
	fscanf(fp,"%d %d %d ",&row,&col,&maxval);
	while(fscanf(fp,"%d%*1[\n]",&val)!=EOF){
		a[index]=val;
		index+=1;
	}
	for(i=index;i<index+5;i++){
		a[i]=-1;
	}
	pgm->row=row;
	pgm->col=col;
	pgm->maxval=maxval;
	pgm->compressed=a;
	pgm->compressed_size=arraysize+5;
	return pgm;
}

//change the color of pixel in given location
bool change_pixel(int* compressed,int location,int newval){
	int count=0;
	int index=-1;
	int indexval;
	int val;
	int i=0;
	int len;
	int start;
	int end;
	while(compressed[i]!=-1 && index==-1){
		if(i%2==0){
			count+=compressed[i];
			if(count>=location){
				index=i;
				indexval=compressed[i];
				val=compressed[i+1];
			}
		}
		i+=1;
	}
	if(val==newval){
		printf("error : same value\n");
		return false;
	}
	start=count-indexval+1;
	end=count;
	printf("%d %d %d\n",start,end,location);
	int tmp[7]={0,0,0,0,0,0,0};
	i=0;

	if(index>=0 && compressed[index-1]==newval && location==start){
		compressed[index-2]+=1;
		compressed[index]-=1;
		printf("array updated\n");
		return true;
	}
	if(compressed[index+3]==newval && location==end){
		compressed[index+2]+=1;
		compressed[index]-=1;
		printf("array updated\n");
		return true;
	}
	if(location>start){
		tmp[i]=location-start;
		tmp[i+1]=val;
		i+=2;
	}
	tmp[i]=1;
	tmp[i+1]=newval;
	i+=2;
	if(location<end){
		tmp[i]=end-location;
		tmp[i+1]=val;
		i+=2;
	}
	len=i;
	shift_right(compressed,index,len-2);
	fill(compressed,tmp,index,len);
	printf("array updated\n");
	return true;
}

//shift the array to the right starting from index val times
void shift_right(int* compressed,int index,int val){
	int i=index;
	int end;
	while(compressed[i]!=-1){
		i+=1;
	}
	end=i;
	for(i=end;i>=index;i--){
		compressed[i+val]=compressed[i];
	}
}
//fill the shifted array 
void fill(int* dst,int* src,int index,int val){
	int i;
	for(i=index;i<index+val;i++){
		dst[i]=src[i-index];
	}
}

//convert array to compressed file
bool array_to_compress(PGM* pgm,char* filename,int rowlen){
	FILE *fp=fopen(filename,"w");
	if(fp==NULL){
		printf("error : memory allocation error\n");
		return false;
	}
	int val;
	int count=1;
	int rowcount=0;
	int i;
	int col=pgm->col;
	i=0;
	int len=0;
	while(pgm->compressed[i]!=-1){
		i+=1;
		len+=1;
	}
	fprintf(fp,"%d %d %d ",pgm->row,pgm->col,pgm->maxval);
	for(i=0;i<len;i++){
		fprintf(fp,"%d ",pgm->compressed[i]);
		rowcount+=1;
		if(rowcount>rowlen){
			fputs("\n",fp);
			rowcount=0;
		}
	}
	free(pgm->compressed);
	free(pgm);
	fclose(fp);
	printf("array converted to compressed file\n");
	return true;
}

//shift array to the left
void shift_left(int* compressed,int index,int val,int size){
	int i=index;
	for(i=index;i<size;i++){
		compressed[i-val]=compressed[i];
	}
}

//returns a list all files in given filetype in the same folder with source code
char**list_of_files(int*list_size,char*filetype){
	char**list=(char**)malloc(50*sizeof(char*));
	DIR *d;
    struct dirent *dir;
    d = opendir(".");
    if (d)
    {
        while ((dir = readdir(d)) != NULL){
        	if(file_control(dir->d_name,filetype,0)==true){
        		char*filename=(char*)malloc(150*sizeof(char));
        		strcpy(filename,dir->d_name);
        		list[*list_size]=filename;
        		*list_size+=1;
			}
        }
        closedir(d);
    }
    return list;
}

//free file list
void free_list(char**list,int listsize){
	int i;
	for(i=0;i<listsize;i++){
		free(list[i]);
	}
	free(list);
}

//print file list
void print_list(char**list,int listsize){
	int i;
	printf("---------------------------------\n");
	for(i=0;i<listsize;i++){
		printf("%-4d%s\n",i+1,list[i]);
	}
	printf("---------------------------------\n");
	
}

//print matrix 
void print_matrix(int** m,int row,int col,int rowlen,bool active){
	if(active==false){
		return;
	}
	int i,j;
	int count=0;
	printf("------------------------------------\n");
	for(i=0;i<col;i++){
		for(j=0;j<row;j++){
			printf("%-5d",m[i][j]);
			count+=1;
			if(count>=rowlen){
				count=0;
				printf("\n");
			}
		}
	}
	printf("------------------------------------\n");
}

//print array 
void print_array(int*array,int len,int rowlen,bool active){
	if(active==false){
		return;
	}
	int i;
	int count=0;
	printf("------------------------------------\n");
	for(i=0;i<len;i++){
		if(array[i]!=-1){
			printf("%-5d",array[i]);
			count+=1;
	    	if(count>=rowlen){
		    	count=0;
			    printf("\n");
	     	}
		}
	}
	printf("\n");
	printf("------------------------------------\n");
}

//print a histogram and save histogram to a txt file
void histogram(int* compressed,char*filename,int maxval,int len){
	FILE *fp=fopen(filename,"w");
	if(fp==NULL){
		printf("error : memory allocation error\n");
		return;
	}
	int c=0;
	double num;
	int i,j;
	int * histogram=(int*)malloc((maxval+1)*sizeof(int));
	for(i=0;i<maxval+1;i++){
		histogram[i]=0;
	}
	
	for(i=0;i<len;i++){
		if(compressed[i]!=-1){
			histogram[compressed[i+1]]+=compressed[i];
			c+=compressed[i];
		}
		i+=1;
	}
	printf("number  count  percent\n");
	for(i=0;i<maxval+1;i++){
		if(histogram[i]>0){
			num=((double)histogram[i]/c)*100;
			fprintf(fp,"%-5d %-8d |",i,histogram[i]);
			printf("%-5d %-8d %c%.2f\t|",i,histogram[i],'%',num);
			for(j=0;j<(int)num;j++){
				fprintf(fp,"/");
				printf("/");
			}
	 		fputc('\n',fp);
	 		printf("\n");
		}
   	}
	free(histogram);
	fclose(fp);
}

bool change_color(PGM* pgm,char* filename,int color,int newcolor,int rowlen){
	FILE *fp=fopen(filename,"w");
	if(fp==NULL){
		printf("error : memory allocation error\n");
		return false;
	}
	fprintf(fp,"%d %d %d ",pgm->row,pgm->col,pgm->maxval);
	int i=0;
	int size=pgm->compressed_size;
	int *compressed;
	int count=0;
	int val;
	int prevval=compressed[1];
	int len=0;
	compressed=pgm->compressed;
	while(i<size){
		if(compressed[i]!=-1){
			val=compressed[i+1];
	    	if(val==color || val==newcolor){
		    	count+=compressed[i];
	    	}else{
			    if(count>0){
					fprintf(fp,"%d %d ",count,newcolor);
					printf("%d\t%d\t",count,newcolor);
					len+=1;
					if(len>rowlen){
						len=0;
						fputc('\n',fp);
					}
					count=0;
				}
				fprintf(fp,"%d %d ",compressed[i],compressed[i+1]);
				printf("%d\t%d\t",compressed[i],compressed[i+1]);
				len+=1;
				if(len>rowlen){
					len=0;
					fputc('\n',fp);
					printf("\n");
				}
		
			
	    	}
		}
		i+=2;
	}
	if(count>0){
		fprintf(fp,"%d %d ",count,newcolor);
		printf("%d\t%d\t",count,newcolor);
	}
	printf("\n");
	fclose(fp);
	return true;
}
