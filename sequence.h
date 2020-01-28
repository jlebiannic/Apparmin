//JLB01
#ifndef _WIN32
#include "f64.h"
#endif


struct structSeq {
  char rec[4];
  char seq[13];
}
seqTab [100000];
int nTailleSeqTab=0;

void IncrementSeqTabSeq(structSeq* pStructSeq);


//Creation des s�quence des records en fonction des lignes lues dans le fichier des s�quences
void AlimenteSeq(char* line, int cpt)
{
 char strReq[100];
 char strSeq[50];
 int n = sscanf(line, "%s%s",strReq,strSeq);
 if (n!=2)
 {
  printf("ERREUR pendant la lecture du fichier des s�quences");
  exit(1);
 }
 strcpy(seqTab[cpt].rec, strReq);
 strcpy(seqTab[cpt].seq, strSeq);

 //IncrementSeqTabSeq(&(seqTab[cpt]));
}

//Recherche d'un s�quence d'un record
structSeq* RechercheSeqTab(char* rec)
{
 for(int i=0; i< nTailleSeqTab; i++)
 {
  if(strcmp(rec, seqTab[i].rec)==0)
  {
   return &(seqTab[i]);
  }
 }

 return NULL;
}
char* RechercheSeq(char* rec)
{

 structSeq *pSeqTab = RechercheSeqTab(rec);

 if(pSeqTab != NULL)
 {
  IncrementSeqTabSeq(pSeqTab);
  return pSeqTab->seq;
 }
 
 //Premi�re fois que l'on rencontre le record
 strcpy(seqTab[nTailleSeqTab].rec, rec);
 strcpy(seqTab[nTailleSeqTab].seq, "000000000001");

 nTailleSeqTab++;
 return seqTab[(nTailleSeqTab-1)].seq;
 
}


//Incrementation de la s�quence d'un record
void IncrementSeqTabSeq(structSeq* pStructSeq)
{
 __int64 lSeq = _atoi64(pStructSeq->seq);

 if (lSeq >= 999999999999)
 {
	printf("Erreur d'incr�mentation, s�quence trop grande pour le record: %s\n",pStructSeq->rec);
	exit(1);
 }

 lSeq++;

 char str64Seq[50];
 _i64toa(lSeq,str64Seq,10);
 sprintf(pStructSeq->seq, "%012s", str64Seq);

}
//void IncrementeSeq(char* rec)
//{
// structSeq *pSeqTab = RechercheSeqTab(rec);
//
// if(pSeqTab != NULL)
// {
//  IncrementSeqTabSeq(pSeqTab);
// }
// else
// {
//  printf("Erreur interne: incrementation des s�quences, record non trouv�: %s\n",rec);
// }
//
//}
