#ifndef _WIN32//JLB01
#include "type64.h" //JLB01
#endif//JLB01

const int nLgDbKey = 12;
const int nLgIdent = 12;
const int nLgDbKeyPacked = 7;
const int nLgOrdrePacked = 7;

const int nLgSchema = 0;
const int nLgArea	= 3;
const int nLgRecord = 3;
const int nLgSet	= 3;
const int nLgFlag	= 1;
const int nLgCalc	= 100; // ADD CALC ZONE
const int nLgFiller = 0;

const int nLgOrdre  = 12;
const char sFlagFLMA[2] = "X";

const char strFinSet[nLgDbKey+1] = "000000000000";


/////////////////////////////
struct Membre {
	//ar sSchema[nLgSchema+1];	//Schema
	char sArea[nLgArea+1];		//Area
	char sRecord[nLgRecord+1];	//Type du Record
	char sSet[nLgSet+1];		//Set
	char sFlagRenum[nLgFlag+1];  //Flag Insertion
	char sFlagsMisc[5*nLgFlag+1];  //Flag Divers
	char sMembre[nLgDbKey+1];	//DBKEY du record
	char sIdent[nLgIdent+1];	//IDENT
	char sIdentSeq[nLgIdent+1];	//IDENT : Num sequence
	char sOwner[nLgDbKey+1];	//DBKEY Owner
	char sIdentOwner[nLgIdent+1];	//IDENT Owner
	char sNext[nLgDbKey+1];		//Next
	char sPrior[nLgDbKey+1];	//Prior
	char sOrdre[nLgOrdre+1]; //Ordre
	char sCalc[nLgCalc+1];		//Cle Calc du pere // ADD CALC ZONE
	//ar sFiller[nLgFiller+1];	//Filler

	//__int64  ordre;					//Num ordre  //JLB01
	//int ordreGlobal;			//Num ordre global au type   //JLB01


	Membre *next;
	Membre *prior;

};

Membre *creationMembre()
{

	Membre* mb = (Membre*) malloc(sizeof(Membre));

	mb->next = NULL;
	mb->prior = NULL;
	//mb->ordre = -1;  //JLB01
	//mb->ordreGlobal = -1;  //JLB01

	//rcpy(mb->sSchema, "sch_vide");
	strcpy(mb->sArea, "999");
	strcpy(mb->sRecord, "999");
	strcpy(mb->sSet, "999");

	strcpy(mb->sFlagsMisc, "FFFFF");
	strcpy(mb->sFlagRenum, "F");

	strcpy(mb->sMembre, "membre_vide");
	strcpy(mb->sIdent, "ident_vide");
	strcpy(mb->sIdentSeq, "idSeq_vide");

	strcpy(mb->sOwner, "owner_vide");
	strcpy(mb->sIdentOwner, "own_id_vide");
	strcpy(mb->sNext, "next_vide");
	strcpy(mb->sPrior, "prior_vide");

	strcpy(mb->sOrdre, "ordre_vide");

	strcpy(mb->sCalc, "calc_vide"); // ADD CALC ZONE
	//rcpy(mb->sFiller, "fil_vide");


	return mb;

}

