/****************************************************************************/
/* IDENTIFICATION                                                           */
/*--------------------------------------------------------------------------*/
/* FICHIER          : apparmin.cpp                                          */
/* VERSION          : 1.0                                                   */
/****************************************************************************/
/* DESCRIPTION                                                              */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Appareillage pour Unix                                                   */
/* REMARQUE IMPORTANTE: apparmin ne fonctionne actuellement qu'avec des		*/
/*                      fichiers Unix (1 seul cararactère à la fin de       */
/*                      la ligne)                                   		*/
/*						car le fichier étant ouvert en binaire la fonction	*/
/*						fread considère qu'il n'y a qu'un seul caractère	*/
/*						à la fin de la ligne (contrairement à un fichier	*/
/*						Windows qui lui possède deux caractères				*/
/*						=> fin de ligne puis retour chariot):				*/
/*						fread(pBuffer, 1, longueurFichier+1, filein).		*/
/*						Pour fonctionner avec un fichier de type Windows	*/
/*						il faudrait écrire:									*/
/*						fread(pBuffer, 1, longueurFichier+2, filein).		*/	
/*																			*/
/*                                                                          */
/****************************************************************************/
/* MAINTENANCES                                                             */
/*--------------------------------------------------------------------------*/
/* Tag   Date     | Auteur    | Description                                 */
/*-----+----------+-----------+---------------------------------------------*/
/*  01 |14/01/2004| JLB       | Ajout du fichier des séquences              */
/*  02 |28/01/2020| JLB       | Réutilisation de la zone "calc"             */
/*     |          |           |  pour compatibilité                         */
/*     |          |           |                                             */
/****************************************************************************/


#include "stdafx.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "hash.h"
#include "membre.h"

#include "sequence.h"//JLB01

Membre* membreDepart = NULL;
Membre* membreFin = NULL;

//Ordre Global
//int nOrdreGlobal = 1;  //JLB01


//Gestion de la rupture
char sLastOwner[nLgDbKey+1] = "";
char sLastSet[nLgSet+1]     = "";
char sOwnerVide[nLgDbKey+1] = "000000000000";
char sSetVide[nLgSet+1]     = "   ";

//Rapport
int nNbLus = 0;
int nNbChaines = 0;

int nNbPeres = 0;
int nNbLusGlobal = 0;
int nNbChainesGlobal = 0;

//Fichiers
#ifdef _WIN32
char strFicIn[100] = "Data\\entreeN.txt";
char strFicOut[100] = "sortie.txt";
char strFicSeq[100] = "seq.txt";//JLB01
char strFicHash[100] = "hash.txt";
char strFicRpt[100] = "rapport.txt";
char strFicRptGlobal[100] = "rptGlob.txt";

#else
char strFicIn[100] = "/cd6/Unload/Res/LHH1S0/tmp1.txt";
char strFicOut[100] = "/cd6/Unload/Res/LHH1S0/appar.txt";
char strFicSeq[100] = "/cd6/Unload/Res/LHH1S0/seq.txt";//JLB01
char strFicHash[100] = "/cd6/Unload/Res/LHH1S0/hash.txt";
char strFicRpt[100] = "/cd6/Unload/Res/LHH1S0/rpt.txt";
char strFicRptGlobal[100] = "/cd6/Res/LHH1S0/rptGlobal.txt";

#endif


//Longueur du fichier d'entree
const int longueurFichier = nLgSchema +
       nLgArea  +
       nLgRecord +
       nLgSet  +
       (nLgFlag*6) +
       (nLgDbKey*7)+
       nLgOrdre+
       nLgCalc+
       nLgFiller;

//Longueur du fichier de sortie
const int longueurFicSortie = nLgSchema +
       nLgArea  +
       nLgRecord +
       nLgSet  +
       (nLgFlag*6) +
       (nLgDbKey*7)+
       nLgOrdre+
       nLgCalc+
       nLgFiller;

FILE* f_out;
FILE* f_hash;
FILE* f_rpt;

  char *pBuffer = new char[longueurFichier+100];

//Booleens
int bRapport = 1;
int bRapportGlobal = 1;
int bHashRpt = 1;

//Extraction des différents champs du membre
int extractChamp(char *pBuf, char* champ, int lg, int& pos)
{
 memcpy (champ, pBuf+pos, lg);
    champ[lg]= '\0';
 //printf("Champ(%i,%i):<%s>, %i", pos, pos+lg, champ, strlen(champ));
 //printf("\n");
 pos=pos+lg;

 return 0;
}


/////////////////////////////
int constructBuffer(char *pBuf, char* champ, int lg, int& pos)
{
 memcpy (pBuf+pos, champ, lg);
 pos=pos+lg;
 return 0;
}


void chainage(Membre *mb, HashTable& ht)
{
 if(strcmp(mb->sNext, strFinSet)==0)
 {
  mb->next = NULL;
  membreFin = mb;
 }
 else
 {
  Membre *MembreNext = (Membre *)hashTableFind(ht, mb->sNext);
  if(MembreNext)
  {
   MembreNext->prior = mb;
   mb->next = MembreNext;

   nNbChaines++;
   nNbChainesGlobal++;
  }
 }

 if(strcmp(mb->sPrior, strFinSet)==0)
 {
  mb->prior = NULL;
  membreDepart = mb;
 }
 else
 {
  Membre *MembrePrior = (Membre *)hashTableFind(ht, mb->sPrior);
  if(MembrePrior)
  {
   MembrePrior->next = mb;
   mb->prior = MembrePrior;

   nNbChaines++;
   nNbChainesGlobal++;
  }

 }

 hashTableInsert(ht, mb->sMembre, mb);

}

int ecritResultats(HashTable& ht)
{

 if (membreDepart == NULL)
	 return 1;

 //ecriture du fichier de sortie
 Membre *mb = membreDepart;
 __int64 nOrdre = 1;
 char sOrdreTmp [nLgOrdre+1]="";
 char sOrdre [nLgOrdre+1]="";

 while (mb)
 {
  
  //mb->ordre = nOrdre; //JLB01

  //mb->ordreGlobal = nOrdreGlobal;  //JLB01
  //Si F/L et Aut/Mand conv de l'ordre Glob en chaine pour num seq
  if (strcmp(mb->sFlagRenum,sFlagFLMA)==0)
  {
   //incrémentation automatique de la séquence
   strcpy(mb->sIdentSeq, RechercheSeq(mb->sRecord));//JLB01 Rq: pour les sets multi-record, la colonne Ordre est toujours utilisée
  }
  
  //Conversion ordre en chaine
  _i64toa(nOrdre,sOrdreTmp,10);
  sprintf(sOrdre, "%012s", sOrdreTmp);

  //JLB01
  //Si F/L et Aut/Mand conv de l'ordre Glob en chaine pour num seq
  //strcpy(mb->sIdentSeq,"000000000000");
  //if (strcmp(mb->sFlagRenum,sFlagFLMA)==0)
  //{
  // sprintf(mb->sIdentSeq, "%012d", mb->ordreGlobal);
  //}

  int pos=0;
  memset( pBuffer, '*', longueurFichier+100);
  //nstructBuffer(pBuffer, mb->sSchema, nLgSchema, pos); //Schema
  constructBuffer(pBuffer, mb->sArea, nLgArea, pos);  //Area
  constructBuffer(pBuffer, mb->sRecord, nLgRecord, pos); //Type du Record
  constructBuffer(pBuffer, mb->sSet, nLgSet, pos);  //Set
  constructBuffer(pBuffer, mb->sFlagRenum, nLgFlag, pos);
  constructBuffer(pBuffer, mb->sFlagsMisc,5*nLgFlag, pos);
  constructBuffer(pBuffer, mb->sMembre, nLgDbKey, pos);
  constructBuffer(pBuffer, mb->sIdent, nLgIdent, pos);
  constructBuffer(pBuffer, mb->sIdentSeq, nLgIdent, pos);
  constructBuffer(pBuffer, mb->sOwner, nLgDbKey, pos);
  constructBuffer(pBuffer, mb->sIdentOwner, nLgIdent, pos);
  constructBuffer(pBuffer, mb->sNext, nLgDbKey, pos);
  constructBuffer(pBuffer, mb->sPrior, nLgDbKey, pos);
  constructBuffer(pBuffer, sOrdre, nLgOrdre, pos);
  constructBuffer(pBuffer, mb->sCalc, nLgCalc, pos); // ADD CALC ZONE
  //nstructBuffer(pBuffer, mb->sFiller, nLgFiller, pos);
  pBuffer[longueurFichier]='\n';
  //printf("W<%s>W\n", pBuffer);

  if (fwrite(pBuffer, 1, longueurFichier+1, f_out) < 0)
  {
   printf("Erreur ecriture: %s\n",strFicOut);
   exit(99);
  }

  nOrdre ++;
  //nOrdreGlobal ++;  //JLB01

//  if (strcmp(mb->sFlagRenum,sFlagFLMA)==0)
//  {
//   IncrementeSeq(mb->sRecord); //JLB01
//  }

  mb = mb->next;

 }


#ifdef _WIN32
 //fprintf(f_out,"=====\n");
#endif

 //Ecriture de la hashtable
 if(bHashRpt)
 {

  hashTablePrint(ht, f_hash);

  int resPrintf = fprintf(f_hash,"\n================\n");
  if (resPrintf <0)
   printf("Erreur ecriture: %s\n",strFicHash);


 }

 //Ecriture du fichier rapport
 if(bRapport)
 {

  int resPrintf = 
	  fprintf(f_rpt,"==>PERE (SET): %s (%s)\n", sLastOwner, sLastSet);
  if (resPrintf <0)
   printf("Erreur ecriture: %s\n",strFicRpt);

  resPrintf = fprintf(f_rpt,"  NB FILS   : %d\n", nNbLus);
  if (resPrintf <0)
   printf("Erreur ecriture: %s\n",strFicRpt);

  resPrintf = fprintf(f_rpt,"  NB CHAINES: %d\n\n", nNbChaines);
  if (resPrintf <0)
   printf("Erreur ecriture: %s\n",strFicRpt);

 }

 return 0;

}

void ecritRapportGlobal()
{
 FILE* f_rpt;

 if ((f_rpt = fopen(strFicRptGlobal, "w")) == 0)
 {
  fprintf(stderr,
   "Probleme d'ouverture en ajout: %s\n", strFicRptGlobal);
  exit(1);
 }


 int resPrintf = fprintf(f_rpt,"  NB PERES/SETS: %d\n", nNbPeres);
 if (resPrintf <0)
  printf("Erreur ecriture: %s\n",strFicRptGlobal);

 resPrintf = fprintf(f_rpt,"  NB FILS      : %d\n", nNbLusGlobal);
 if (resPrintf <0)
  printf("Erreur ecriture: %s\n",strFicRptGlobal);

 resPrintf = fprintf(f_rpt,"  NB CHAINES   : %d\n\n", nNbChainesGlobal);
 if (resPrintf <0)
  printf("Erreur ecriture: %s\n",strFicRptGlobal);

 fclose(f_rpt);

}



//Lecture du fichier
int lectureFichier(FILE* filein, Membre* mb)
{

 //ar *pBuffer = new char[longueurFichier+100];
 memset( pBuffer, '*', longueurFichier+100);

 int numRead = fread(pBuffer, 1, longueurFichier+1, filein);
 int pos =0;
 //uffer[longueurFichier]='\0';
 if(numRead > 0)
 {
  //tractChamp(pBuffer, mb->sSchema, nLgSchema, pos); //Schema
  extractChamp(pBuffer, mb->sArea, nLgArea, pos);
  extractChamp(pBuffer, mb->sRecord, nLgRecord, pos);
  extractChamp(pBuffer, mb->sSet, nLgSet, pos);
  extractChamp(pBuffer, mb->sFlagRenum, nLgFlag, pos);
  extractChamp(pBuffer, mb->sFlagsMisc, 5*nLgFlag, pos);
  extractChamp(pBuffer, mb->sMembre, nLgDbKey, pos);
  extractChamp(pBuffer, mb->sIdent, nLgIdent, pos);
  extractChamp(pBuffer, mb->sIdentSeq, nLgIdent, pos);
  extractChamp(pBuffer, mb->sOwner, nLgDbKey, pos);
  extractChamp(pBuffer, mb->sIdentOwner, nLgIdent, pos);
  extractChamp(pBuffer, mb->sNext, nLgDbKey, pos);
  extractChamp(pBuffer, mb->sPrior, nLgDbKey, pos);
  extractChamp(pBuffer, mb->sOrdre, nLgOrdre, pos);
  extractChamp(pBuffer, mb->sCalc, nLgCalc, pos); // ADD CALC ZONE
  //tractChamp(pBuffer, mb->sFiller, nLgFiller, pos); //Filler
  //printf("\nR<%s>R\n", pBuffer);

  return 1;
 }
 return 0;
}

// Lecture d'un fichier et traitement de chaque ligne
void lectureFichierEtChainage(FILE* filein, HashTable& ht)
{
 int bFirstTime = 1;

 Membre* mb = creationMembre();
 while( lectureFichier (filein, mb) )
 {

  nNbLus++;
  nNbLusGlobal++;

  if (!bFirstTime &&
    ((strcmp(sLastOwner, mb->sOwner)!=0) ||
     (strcmp(sLastSet, mb->sSet)!=0) ||
     (strcmp(sOwnerVide, mb->sOwner)==0)||
     (strcmp(sSetVide, mb->sSet)==0))
   )
  {
   nNbLus--;

   //ecriture des resultats
   if(ecritResultats(ht) != 0)
   {
	 fprintf(stderr,
	 "Le membre de départ est vide ! Dernier membre accédé: %s\n", mb->sMembre);
	 fclose(filein);
     exit(98);
   }

   //Destruction de l'ancienne table
   hashTableDestroy(ht);
   membreDepart = NULL;

   // Initialisation de la table
   hashTableInit(ht, MOD);

   //Initialisations diverses
   nNbLus = 0;
   nNbChaines = 0;

   //JLB01
   /*if(strcmp(sLastSet, mb->sSet)!=0)
   {
    nOrdreGlobal = 1;
   }*/


   strcpy(sLastOwner, mb->sOwner);
   strcpy(sLastSet, mb->sSet);

   nNbPeres++;
   nNbLus++;
  }

  if(bFirstTime)
  {
   bFirstTime = 0;
   strcpy(sLastOwner, mb->sOwner);
   strcpy(sLastSet, mb->sSet);
   nNbPeres++;
  }

  chainage(mb, ht); //rech si les next/prior existent si oui chainage

  mb = creationMembre();
 }

 //ecriture des resultats
 if(ecritResultats(ht) != 0)
 { 
  fprintf(stderr,
  "Le membre de départ est vide ! Dernier membre accédé: %s\n", mb->sMembre);
  fclose(filein);
  exit(98);
 }


 //Destruction de l'ancienne table
 hashTableDestroy(ht);
 membreDepart = NULL;

}

void initFichier(char *strFileName, FILE **f_)
{

#ifdef _WIN32
 if ((*f_ = fopen(strFileName, "w")) == 0)
#else
 // ((*f_ = fopen(strFileName, "w")) == 0)
 if ((*f_ = fopen(strFileName, "wb, recfm=*,type=record")) == 0)
#endif
 {
  fprintf(stderr,
   "Probleme d'ouverture en ecriture: %s\n", strFileName);
  exit(1);
 }


}

void dechargeFichierSequence()//JLB01
{
 FILE* f_seq;
#ifdef _WIN32
 if ((f_seq = fopen(strFicSeq, "w")) == 0)
#else
 if ((f_seq = fopen(strFicSeq, "wb, recfm=*,type=record")) == 0)
#endif
 {
  fprintf(stderr,
   "Probleme d'ouverture en ecriture: %s\n", strFicSeq);
  exit(1);
 }

 for(int i=0; i< nTailleSeqTab; i++)
 {
  fprintf(f_seq,"%s\t%s\n",seqTab[i].rec,seqTab[i].seq);
 }

 fclose (f_seq);
}

void chargeFichierSequence()//JLB01
{
 FILE* f_seq;
#ifdef _WIN32
 if ((f_seq = fopen(strFicSeq, "r")) == 0)
#else
 if ((f_seq = fopen(strFicSeq, "rb, type=record")) == 0)
#endif
 {
  fprintf(stderr, "Fichier des séquences absent: %s\n", strFicSeq);
  return;
 }

 char line[1000];
 int cpt=0;

 while( !feof( f_seq ) )
 {
   if(fgets(&(line[0]),1000,f_seq))
   {
    AlimenteSeq(&(line[0]), cpt);
    cpt++;
   }
 }

 nTailleSeqTab=cpt;

 fclose (f_seq);

}

// Programme principal
int main(int argc, char* argv[])
{

 if (argc != 7)//JLB01
 {
   printf("Erreur d'arguments !: FicIn, FicOut, FicSeq, FicHash, FicRpt, FicRptGlobal");//JLB01
   exit(0);
 }

 strcpy( strFicIn,  argv[1]);
 strcpy( strFicOut,  argv[2]);
 strcpy( strFicSeq,  argv[3]);//JLB01
 strcpy( strFicHash,  argv[4]);
 strcpy( strFicRpt,  argv[5]);
 strcpy( strFicRptGlobal,  argv[6]);

 HashTable ht;

 // Initialisation de la table
 hashTableInit(ht, MOD);

 // Initialisation du fichier de sortie
 initFichier(strFicOut, &f_out);

 //JLB01 Chargement du fichier des séquences en mémoire
 chargeFichierSequence();


 // Initialisation du fichier hashtable
 initFichier(strFicHash, &f_hash);

 // Initialisation du fichier rapport
 initFichier(strFicRpt, &f_rpt);


 //Traitement du fichier d'entrée
 FILE* filein;
#ifdef _WIN32
 if ((filein = fopen(strFicIn, "r")) == 0)
#else
 if ((filein = fopen(strFicIn, "rb, type=record")) == 0)
#endif
 {
  fprintf(stderr, "Probleme d'ouverture: %s\n", strFicIn);
  exit(1);
 }


 lectureFichierEtChainage(filein, ht);

 fclose(filein);

 //JLB01 Déchargement du fichier des séquences
 dechargeFichierSequence();


 //Fermeture des autres fichiers
 fclose(f_out);
 fclose(f_hash);
 fclose(f_rpt);


 //Ecriture du fichier rapport global
 if(bRapportGlobal)
 {
  ecritRapportGlobal();
 }

 return 0;
}



