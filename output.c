#include "output.h"
#include "array.h"
#include "chromosome.h" 

#include <stdio.h>

void print_class(FILE *fp, Varray *classes, int room);
void html_add_start(FILE *fp);
void html_add_end(FILE *); 

void print(Chromosome *c, const char *path){
  Chromosome* best =  c; 
  FILE *fp =fopen(path, "w+"); 
  if (!fp) {
    perror("Could not open file for output:"); 
    return;
  }
  int day, period, room ; 

  html_add_start(fp); 
  int nr = cardHashS(global.rooms);
      for (period = 0 ; period < DAYS_HOURS ; period++){
      fprintf(fp,"<tr class=\"day\">");
      for (day = 0 ; day < DAYS_NUM ; day++){
	fprintf(fp, "<td>"); 
	for (room = 0 ; room <  nr; room++){
	  int pos = day * nr * DAYS_HOURS + room * DAYS_HOURS + period; 
	  Varray *classes = best->slots[pos].classes_array; 
	  if (Varray_length(classes) > 0){
	    if (Varray_length(classes) > 1){
	      fprintf(fp,"#conflict#"); 
	    }
	    print_class(fp,classes, room);
	    fprintf(fp, "<hr>"); 
	  }
	}
	fprintf(fp, "</td>"); 
      }
      fprintf(fp,"</tr>");
  }
  html_add_end(fp); 
  fclose(fp); 
  printf("Printed best\n"); 
}

void print_students(FILE *, Varray * students);

void print_class(FILE *fp, Varray *classes, int room){
  int i; 
  Class *c; 
  int has_next; 
  char room_id[32]; 
  sprintf(room_id, "%d", room); 
  
  Room *r = searchHashS(global.rooms, room_id); 
  for (i = 0 , c = Varray_get(classes,i), has_next = ((i+1) <  Varray_length(classes)) ; 
       i < Varray_length(classes); 
       i ++ , c = Varray_get(classes,i), has_next = ((i+1) <  Varray_length(classes))){
    fprintf(fp,"Class %d ", c->class_id); 
    fprintf(fp, "with teacher %s for class", c->teacher_id); 
    print_students(fp,c->students); 
    fprintf(fp, " @  %s", r->idd); 
    if (has_next){
      fprintf(fp, " # "); 
    }
  }
}

void print_students(FILE *fp, Varray * students){
  int i; 
  char *s; 
  int has_next; 
  if (Varray_length(students) > 1 ){
    fprintf(fp, "("); 
  }
  
  for (i = 0 , s = Varray_get(students,i), has_next = ((i+1) <  Varray_length(students)) ; 
       i < Varray_length(students); 
       i ++ , s = Varray_get(students,i), has_next = ((i+1) <  Varray_length(students))){
    fprintf(fp, "%s", s);
    if ( has_next)
      fprintf(fp, ","); 
  }

  if (Varray_length(students) > 1 ){
    fprintf(fp, ")"); 
  }
}

void html_add_start(FILE *fp){
const char *head = "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\">\n<head>\n<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\" />\n<link href=\"css/screen.css\" rel=\"stylesheet\" type=\"text/css\" media=\"screen, projection\" />\n<link href=\"css/print.css\" rel=\"stylesheet\" type=\"text/css\" media=\"print\" />\n<title>Calendar</title>\n</head>\n<body>\n<table cellspacing=\"0\", border=1>\n\t<caption>January 2079</caption>\n\t<colgroup>\n\t\t<col class=\"Mon\" />\n\t\t<col class=\"Tue\" />\n\t\t<col class=\"Wed\" />\n\t\t<col class=\"Thu\" />\n\t\t<col class=\"Fri\" />\n\t</colgroup>\n\t<thead>\n\t\t<tr>\n\t\t\t<th scope=\"col\">Monday</th>\n\t\t\t<th scope=\"col\">Tuesday</th>\n\t\t\t<th scope=\"col\">wednesday</th>\n\t\t\t<th scope=\"col\">Thursday</th>\n\t\t\t<th scope=\"col\">Friday</th>\n\t\t</tr>\n\t</thead><tbody>"; 
 fprintf(fp, "%s\n", head); 
 
}

void html_add_end(FILE *fp){
  const char  *end = "\t</tbody>\n</table>\n</body>\n</html>";
  fprintf(fp, "%s\n", end); 
}

