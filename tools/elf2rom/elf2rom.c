 
#include <stdio.h>
#include <libelf.h>
// THIS IS TEMPORARY TO AVOID A VERSION CONFLICT BETWEEN GNU LIBELF AND BSD LIBELF ON FREEBSD
//#include </usr/local/include/libelf/libelf.h>
#ifdef __GNUC__
#include <unistd.h>
#ifndef O_BINARY
#define O_BINARY 0
#endif
#else
#include <io.h>
#endif
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>


#define DROMLIB_SYMBOL "__dromlib_version"






//! Internal structure of an elf file

typedef struct
{
    // File descritor
    int             fd;
    Elf32_Ehdr     *ehdr;
    Elf32_Shdr     *shdr;
    Elf            *elf;
    Elf_Scn        *scn;
    Elf_Data       *data;
} Elf_file;

typedef struct 
{
    unsigned int index;
    char name[20];
    unsigned int type;
    unsigned int address;
    unsigned int size;
    unsigned char entsize;
    unsigned char flags;
    unsigned char link;
    unsigned int info;
    unsigned char align;
    int offset;
} elf_Section_Table_Entry;



typedef struct
{
    char        *symbol;
    unsigned    addr;
} ROMEntry;

ROMEntry *ROMTable;
int ROMTableSize = 0;

#define ROM_prefix "_ROM_"
#define OUTFILE "romlib.bin"
#define LISTFILE "romentries.list"

void libelf_failure()
{
    fprintf(stderr, "libelf::%s\n", elf_errmsg(elf_errno()));
    exit(1);
}

void _failure(char *msg)
{
	fprintf(stderr,"elf2rom::%s\n",msg);
	exit(-1);
}

/* Given Elf header, Elf_Scn, and Elf32_Shdr 
 * generate the symbol table 
 */
void
generate_symbol_table(Elf *elf, Elf_Scn *scn, Elf32_Shdr *shdr)
 {
      Elf_Data *data;
      char *name;
      data = 0;
      int count = 0;
      if ((data = elf_getdata(scn, data)) == 0 || data->d_size == 0){
          /* error or no data */
          fprintf(stderr,"Section had no data!\n");
               exit(-1);
      }
      
      Elf32_Sym *esym = (Elf32_Sym*) data->d_buf;
      Elf32_Sym *lastsym = (Elf32_Sym*) ((char*) data->d_buf + data->d_size);
     
      // First pass, count the relevant entries ...
      
      for (; esym < lastsym; esym++)
          if (esym->st_value  && (ELF32_ST_TYPE(esym->st_info)== STT_FUNC)) { 
              
                name = elf_strptr(elf,shdr->sh_link , (size_t)esym->st_name);
                if(!name)
                    libelf_failure();  
                if(strstr(name,ROM_prefix) == name) 
                    count++;
           
        }
       
      ROMTable = (ROMEntry *) calloc(ROMTableSize=count,sizeof(ROMEntry));
       
      esym = (Elf32_Sym*) data->d_buf;
      lastsym = (Elf32_Sym*) ((char*) data->d_buf + data->d_size);
      
      int c = 0;
      int prefix_len = strlen(ROM_prefix);
      char buf[255];
    
      // second pass, generate ROM Table entries ...
        
      for (; esym < lastsym; esym++)
          if (esym->st_value  && (ELF32_ST_TYPE(esym->st_info)== STT_FUNC)) { 
              
                name = elf_strptr(elf,shdr->sh_link , (size_t)esym->st_name);
                if(!name)
                    libelf_failure();  
                if(strstr(name,ROM_prefix) == name) {
                    ROMTable[c].symbol = strdup(strcpy(buf,(char *)((unsigned long) name + prefix_len)));
                    ROMTable[c].addr   = (unsigned int) esym->st_value;
                    c++;
                }
           
       }
}

int
check_for_symtab(Elf *elf)
{

     Elf_Scn* section = 0;
     while ((section = elf_nextscn(elf, section)) != 0) {
        Elf32_Shdr *shdr;
        if ((shdr = elf32_getshdr (section)) != 0) {
                if (shdr->sh_type == SHT_SYMTAB) {
                    generate_symbol_table(elf, section, shdr);
                    return 1;
                }
         }
     }
     
     return 0;
 }
 
 int
 ROMEntryCmp(const void *a, const void *b)
 {
     ROMEntry *_a,*_b;
     
     _a = (ROMEntry *) a;
     _b = (ROMEntry *) b;
     
     return _a->addr - _b->addr;
 }
 
 void
 print_romentries(int sorted)
 {
    int i;
    FILE *f;
    if(!(f = fopen(LISTFILE,"w")))
        _failure("Can't open listfile for writing.\n");
    
    if(sorted)
        qsort(ROMTable,ROMTableSize,sizeof(ROMEntry),ROMEntryCmp);
    
    for (i = 0; i < ROMTableSize; i++)
        fprintf(f,"%-40s = 0x%08x ;\n",ROMTable[i].symbol,ROMTable[i].addr);
    
    fclose(f);
}

int
dump_sections(FILE *out, Elf_file *elf_file)
{
    
    Elf_Scn *section;
    Elf_Data *data;
         
    elf_Section_Table_Entry entry;
    
    elf_file->ehdr = elf32_getehdr(elf_file->elf);
    section = elf_getscn(elf_file->elf, elf_file->ehdr->e_shstrndx);
    data = elf_getdata(section, NULL);
    
    int _copy_flag;
    int text_size = 0;
    int file_offset=0;
    int cnt;
    
    //now scan the sections:
    for (cnt = 1, elf_file->scn = NULL; (elf_file->scn = elf_nextscn(elf_file->elf, elf_file->scn)); cnt++) 
    {
        elf_file->shdr = elf32_getshdr (elf_file->scn);

        if (elf_file->shdr != 0) 
        {
            entry.index=cnt;
            strcpy( entry.name, (char*) data->d_buf + elf_file->shdr->sh_name);
            entry.type = elf_file->shdr->sh_type;
            entry.address = elf_file->shdr->sh_addr;
            entry.size = elf_file->shdr->sh_size;
            entry.entsize = elf_file->shdr->sh_entsize;
            entry.flags = elf_file->shdr->sh_flags;
            entry.link = elf_file->shdr->sh_link;
            entry.info = elf_file->shdr->sh_info;
            entry.align = elf_file->shdr->sh_addralign;

            _copy_flag = 0;
            
            // THESE SECTIONS ARE SPECIFIC TO NEWRPL FIRMWARE


            if (!( strcmp(entry.name,".text")
                   && strcmp(entry.name,".rodata")
                   && strcmp(entry.name,".preamble")
                   && strcmp(entry.name,".codepreamble")
                   && strcmp(entry.name,".romobjects")
                   && strcmp(entry.name,".romlink")
                   ))
            {
                //create the binary files (word aligned)
                text_size += entry.size;
                //text_pos = entry.address;
                _copy_flag = 1;
            }
            
            if (_copy_flag)
            {
               
                char *_data = (char*) (elf_getdata(elf_file->scn, NULL)->d_buf);

                if(!file_offset) file_offset=entry.address;

                if(file_offset!=entry.address) {
                    if(file_offset>entry.address) {
                        printf("Error: Bad section alignment\n");
                    }
                    if(file_offset<entry.address) {
                        // NEEDS PADDING!
                        int i;
                        for(i=0; i < (entry.address-file_offset); i++)
                            fwrite("\0",1,1,out);
                        file_offset=entry.address;
                    }

                }


                fwrite(_data,entry.size,1,out);
                file_offset+=entry.size;

                // Pad if not properly aligned
                int i,rest4 = entry.size % 4;
                if (rest4)
                    for(i=0; i < (4-rest4); i++)
                    { fwrite("\0",1,1,out); ++file_offset; }
                    
            }
        }
    }        

    
    return text_size;
}

void
dump_romlib_version(FILE *out, int version)
{
	char temp[2];
	temp[1]=0;
	int f;
	for(f=0;f<4;++f)
	{
		temp[0]=version&0xff;
        fwrite(temp,1,1,out);
        version>>=8;
	}
}

int
get_romlib_version(Elf *elf)
 {
      Elf_Scn *scn;
	  Elf_Data *data;
      Elf32_Shdr *shdr;
      Elf32_Ehdr *ehdr;
	  char *name;
      data = 0;
      int count = 0;
	  long dromlib_addr=-1;
	  int romlib_version=-1;

   if ((ehdr = elf32_getehdr(elf)) == 0)
   {
   _failure("Can't get ELF headers.\n");
    }


	
// GET SYMBOL TABLE SECTION  
	  
	 scn=NULL;
     while ((scn = elf_nextscn(elf, scn)) != NULL) {
        if ((shdr = elf32_getshdr (scn)) != 0) {
                if (shdr->sh_type == SHT_SYMTAB) {
                    break;
                }
         }
     }
     
	 if(scn==NULL) return romlib_version;
	  
// FIND CONFIGURATION SYMBOLS	  
	  
      if ((data = elf_getdata(scn, NULL)) == 0 || data->d_size == 0)	return romlib_version;
  
      Elf32_Sym *esym = (Elf32_Sym*) data->d_buf;
      Elf32_Sym *lastsym = (Elf32_Sym*) ((char*) data->d_buf + data->d_size);
     
        
      for (; esym < lastsym; esym++)
          if (esym->st_value) { 
              
                name = elf_strptr(elf,shdr->sh_link , (size_t)esym->st_name);
                if(!name)
                    libelf_failure();  
					
				
                if(!strcmp(name,DROMLIB_SYMBOL)) 
					dromlib_addr=esym->st_value;
				
           
       }
	   
// FIND .config section

	  
	 scn=NULL;
     while ((scn = elf_nextscn(elf, scn)) != NULL) {
        if ((shdr = elf32_getshdr (scn)) != 0) {
		
				name=elf_strptr(elf,ehdr->e_shstrndx , (size_t)shdr->sh_name);
				if(name)
				{ 
				if (!strcmp(name,".config"))  break;
				}
                
         }
     }
	   
	 if(scn==NULL) return romlib_version;
	  
// FIND CONFIGURATION SYMBOLS WITHIN .config SECTION  
	  
      if ((data = elf_getdata(scn, NULL)) == 0 || data->d_size == 0)	return romlib_version;
 	   
	  if(dromlib_addr!=-1 && (dromlib_addr-shdr->sh_addr)<data->d_size) romlib_version=*((unsigned int *) ((char *)data->d_buf+(dromlib_addr-shdr->sh_addr)));


	return romlib_version;
}










int
main(int argc,char **argv)
{

	char *inp;
	Elf_file elf_file;

	if(argc > 1)
		inp = *++argv;
	else {
		fprintf(stderr,"No input file provided.\n");
		return(-1);
	}

	if (elf_version(EV_CURRENT) ==EV_NONE)
        _failure("Libelf out of date.");

	if ((elf_file.fd = open(inp,O_RDONLY | O_BINARY)) < 0)
		_failure("Can't open input file.\n");

	if ((elf_file.elf = elf_begin(elf_file.fd, ELF_C_READ, NULL)) == NULL)
		libelf_failure();

	/* Check if Symbol Table exists */
    
    if(!check_for_symtab(elf_file.elf))
        _failure("No symbol table found.\n");
    
//    int rlibver=get_romlib_version(elf_file.elf);
//    if(rlibver<0) 
//        _failure("No valid romlib version found.\n");
    
//    printf("Romlib version = %c%c%c%c\n",(rlibver&0xff),((rlibver>>8)&0xff),((rlibver>>16)&0xff),((rlibver>>24)&0xff));
    
    print_romentries(1);
    
    // Dump .text & .rodata sections
    
    FILE *out;
    
	char *p,outfile[256];
	
	strcpy(outfile,inp);
	p = (char *)((unsigned long)outfile + strlen(outfile) - 1);
	while (*p && *p != '.') p--;
	if(*p == '.') {
		*p = '\0';
		strcat(outfile,".bin");
	}
	else
		strcpy(outfile,OUTFILE);

	//printf("DEBUG: outfile=%s\n",outfile);

		
    if(!(out = fopen(outfile,"wb")))
        _failure("Can't open target file for writing.\n");
//    dump_romlib_version(out,rlibver);
    dump_sections(out,&elf_file);
    
    fclose(out);
    close(elf_file.fd);
	
	printf("Binary image written to \"%s\"\n",outfile);
	

	return 0;
}
