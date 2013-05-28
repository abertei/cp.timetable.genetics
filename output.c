#include "output.h"
#include "array.h"
#include <string.h>

#include "chromosome.h" 

#include <stdio.h>

void print_class(FILE *fp, Varray *classes, int room, int pos, Chromosome *c);
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
	    print_class(fp,classes, room, pos, best);
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

void open_class(FILE *fp, Chromosome *cromo, Class *c){
  char phrase[10000] ="";
  // coordinate of time-space slot
  int number_of_rooms = cardHashS(global.rooms); 
  int daySize = DAYS_HOURS * number_of_rooms;

  int *array = cromo->classes_start; 
  int c_start = array[c->class_id]; 
  int p = c_start; 
  int day = p / daySize;
  int time = p % daySize;
  int room = time / DAYS_HOURS;
  char room_str[15]; 
  sprintf(room_str, "%d", room); 
  time = time % DAYS_HOURS;
  int dur = c->periods;		

  // check for room overlapping of classes

  int period;
  for(period = dur - 1; period >= 0; period-- ){
    //TODO - varray length semantics? 
    if( Varray_length(cromo->slots[ p + period ].classes_array) > 1 ){
      strcat(phrase, "Room overlap."); 
      break;
    }
  }
  Class* cc = c; 
  Room* r = (Room *) searchHashS(global.rooms, room_str);
  // does current room have enough seats
  if  (r->seats < cc->seats) {
    strcat(phrase, " Not Enough Seats."); 
  }

  // does current room have computers if they are required
  int  b = cc->type != LAB || ( cc->type == LAB && r->type == LAB );
  if (!b) {
    strcat(phrase, " Incorrect room type.");
  }
  int po = 0, go = 0;
  int room_index; 
  int dayt; 
  // check overlapping of classes for professors and student groups
  for( room_index = number_of_rooms, dayt = day * daySize + time ;
       room_index > 0; 
       room_index--, dayt += DAYS_HOURS ){
    // for each hour of class
    int hour; 
    for( hour = dur - 1; hour >= 0; hour-- ){
      // check for overlapping with other classes at same time
      Varray  * cl = cromo->slots[dayt + hour].classes_array; 
      int indexx;
      Class *it;
      for ( indexx = 0, it = Varray_get(cl, indexx) ; indexx < Varray_length(cl) ; indexx++, it = Varray_get(cl, indexx)){
	if( cc != it ){
	  // professor overlaps?
	  if( !po && (strcmp(cc->teacher_id,it->teacher_id) ==0) ){
	    strcat(phrase," Professor Overlap."); 
	    break; 
	  }
	  // student group overlaps?
	  if( !go && group_overlap(cc, it )){
	    strcat(phrase, " Group Overlap."); 
	    go = 1;
	  }
	}
      }
    }
 }
  if (strlen(phrase) > 0){
    fprintf(fp, "<p title=\"%s\", style=\"background-color:red;\">", phrase); 
  }
  else {
    fprintf(fp, "<p>"); 
  }
}



void print_class(FILE *fp, Varray *classes, int room, int pos, Chromosome *cromo){
  int i; 
  Class *c; 
  int has_next; 
  char room_id[32]; 
  sprintf(room_id, "%d", room); 
  
  Room *r = searchHashS(global.rooms, room_id); 
  for (i = 0 , c = Varray_get(classes,i), has_next = ((i+1) <  Varray_length(classes)) ; 
       i < Varray_length(classes); 
       i ++ , c = Varray_get(classes,i), has_next = ((i+1) <  Varray_length(classes))){ 
    open_class(fp, cromo, c); 
    fprintf(fp,"Class %d ", c->class_id); 
    fprintf(fp, "with teacher %s for group (%d): ", c->teacher_id, c->seats); 
    print_students(fp,c->students); 
    Room *rrr=    searchHashS(global.rooms, r->id); 
    if (rrr != NULL) {
      fprintf(fp, " @  %s (%d)", r->idd, rrr->seats); 
    }
    else{
      fprintf(fp, " @  %s", r->idd); 
    }
  }
  fprintf(fp, "</p>"); 
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

