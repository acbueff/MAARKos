// tool.c, 159

#include "spede.h"
#include "type.h"
#include "extern.h"

void MyBzero(char *p, int size) {

	while(size--){
		*p++ = 0;
	}


}

/**
adding pid to head of queue
*/
void EnQ(int pid, q_t *p) {
// show error msg and return if queue's already full
    if(p->size == 0){//check if it is empty
        p->q[p->head] = pid;
        p->tail++;
		if(p->tail == 20){
			p->tail = 0;
		}
		p->size++;
    }
    else{
         if(p->size < 20){
			p->q[p->tail] = pid;
			p->tail++;
		if(p->tail == 20){
			p->tail = 0;
		}
            p->size++;
         }
         else{
            //error msg for already full
            cons_printf("queue array is full\n");
            return;
         }

    }
}

int DeQ(q_t *p) { // return -1 if q is empty

    int h_pid;//value to return

    if(p->size == 0){
        cons_printf("queue array is empty \n");
        return -1;
    }

    h_pid = p->q[p->head];
	p->head++;
	if(p->head == 20){
		p->head = 0;
	}
    p->size--;
    return h_pid;


}

void MsgEnQ(msg_t *p,mbox_t *q){

	if(q->size == 20){
		cons_printf("mbox is FULL\n");
		return;
	}

	if(q->size == 0){//check if it is empty
        q->msg[q->head] = *p;
        q->tail++;
		if(q->tail == 20){
			q->tail = 0;
		}
		q->size++;
    }
    else{
        if(q->size < 20){
			q->msg[q->tail] = *p;
			q->tail++;
		    if(q->tail == 20){
				q->tail = 0;
		    }
            q->size++;
         }
    }

}
msg_t* MsgDeQ(mbox_t *p){
	msg_t *msg;

	if(p->size == 0){
		cons_printf("mbox is empty\n");
		return 0;
	}
	msg = &p->msg[p->head];
	p->head++;
	if(p->head == 20){
		p->head = 0;
	}
    p->size--;
    return msg;
}

void MyStrcpy(char *dest, char *src){
unsigned i;
  for (i=0; src[i] != '\0'; ++i){
    dest[i] = src[i];
  }
  dest[i] = '\0';
}

int MyStrlen(char *str){
	int i;
    for (i = 0; str[i] != '\0'; i++) ;
    return i;
}
int MyStrcmp(char *str1, char *str2, int size){
	while(size--){
        if(*str1++!=*str2++){
            return 1;//*(unsigned char*)(str1 - 1) - *(unsigned char*)(str2 - 1);
		}
	}
	return 0;

}

void MyMemcpy(char *dest, char *src, int size){
   // Typecast src and dest addresses to (char *)
   char *csrc = (char *)src;
   char *cdest = (char *)dest;

   int i;
   // Create a temporary array to hold data of src
   char temp[size];

   // Copy data from csrc[] to temp[]
   for (i=0; i<size; i++)
       temp[i] = csrc[i];

   // Copy data from temp[] to cdest[]
   for (i=0; i<size; i++)
       cdest[i] = temp[i];
}
