//PSO PROGRAM
/*PROBLEMA:
   MAXIMIZAR LA FUNCION:
   f(x,y)=50-(x-5)^2-(y-5)^2
*/
#include <stdio.h>
#include <stdlib.h>
#include<time.h>
//DEFINICION DE LA ESTRUCTURA PARTICULA
typedef struct{
                       float* Xi;   //POSICION
                       float* Vi;   //VELOCIDAD
                       float* Pi;   //MEJOR POSICION
                       float XFit; //VALOR DE FITNESS DE LA POSICION
                       float PFit; //VALOR DE FITNESS DE LA MEJOR POSICION
                    }PARTICULA;
//DEFINICION DE LA ESTRUCTURA PARA EL ENJAMBRE
typedef struct{
                       unsigned int Nparticulas;
                       unsigned int Nparametros;
                       unsigned int idGbest;             // id global best
                       float Vmin;                            //VELOCIDAD MINIMA
                       float Vmax;                           //VELOCIDAD MAXIMA
                       float C1;                              //COEFICIENTE DE ACELERACION
                       float C2;                              //COEFICIENTE DE ACELERACION
                       PARTICULA* Enj;
                     }ENJAMBRE;

ENJAMBRE* CrearEnjambre(const unsigned int Nparticulas,const unsigned int Nparametros );
void InicializarEnjambre(ENJAMBRE* pEnj,const float LInf, const float LSup, const float C1, const float C2, const float Vinf, const float Vsup);
void ShowParticula(ENJAMBRE* pEnj, const unsigned int i);
void ShowEnjambre(ENJAMBRE* pEnj);
void EliminarEnjambre(ENJAMBRE* pEnj);
void EvaluarEnjambre(ENJAMBRE* pEnj);
void InicializarMejores(ENJAMBRE* pEnj);
void ActualizarVelocidad(ENJAMBRE* pEnj);
void ActualizarPosicion(ENJAMBRE* pEnj);
void ActualizarMejores(ENJAMBRE* pEnj);

ENJAMBRE* CrearEnjambre(const unsigned int Nparticulas,const unsigned int Nparametros )
{
    unsigned int k;
    ENJAMBRE* ptr;
    //ASIGNAR MEMORIA PARA LA ESTRUCTURA ENJAMBRE
    ptr=(ENJAMBRE*)malloc(sizeof(ENJAMBRE));
    if(ptr==NULL)
    {
        printf("ERROR AL ASIGNAR MEMORIA A LA ESTRUCTURA ENJAMBRE");
        exit(0);
    }
    //INICIALIZAR LA ESTRUCTURA
    //ASIGNA NUMERO DE PARTICULAS
    ptr->Nparticulas=Nparticulas;
    //ASIGNA NUMERO DE PARAMETROS
    ptr->Nparametros=Nparametros;
    //ASIGNA MEMORIA PARA LAS PARTICULAS
    ptr->Enj=(PARTICULA*)malloc(sizeof(PARTICULA)*Nparticulas);
    if(ptr->Enj==NULL)
    {
        printf("ERROR AL ASIGNAR MEMORIA A LAS PARTICULAS");
        exit(0);
    }
    //ASIGNAR MEMORIA A CADA PARTICULA
    for(k=0;k<Nparticulas;k++)
    {
        ptr->Enj[k].Xi=(float*)malloc(sizeof(float)*Nparametros);
        ptr->Enj[k].Vi=(float*)malloc(sizeof(float)*Nparametros);
        ptr->Enj[k].Pi=(float*)malloc(sizeof(float)*Nparametros);
    }
    return(ptr);
}

void InicializarEnjambre(ENJAMBRE* pEnj,const float LInf, const float LSup, const float C1, const float C2, const float Vinf, const float Vsup)
{
    unsigned int i,k;
    float r;
    //PARA TODAS LAS PARTICULAS
    for(i=0;i<pEnj->Nparticulas;i++)
         for(k=0;k<pEnj->Nparametros;k++)
        {
           r=((double)rand()/RAND_MAX);             //Numero de 0 a 1
           r=(LSup-LInf)*r+LInf;                              //Numero de LInf a LSup
           pEnj->Enj[i].Xi[k]=r;                                // El valor de r se le da como primera posicion
           pEnj->Enj[i].Vi[k]=0;                               // El valor de 0 se agrega porque suponemos que no tenemos velocidad
           pEnj->Enj[i].Pi[k]=r;                                // El valor de r es la unica mejor posicion que se conoce
        }
        pEnj->C1=C1;
        pEnj->C2=C2;
        pEnj->Vmin=Vinf;
        pEnj->Vmax=Vsup;
}

void ShowParticula(ENJAMBRE* pEnj, const unsigned int i)
{
    unsigned int k;
    printf("\nX[%u]: ",i);
    for(k=0;k<pEnj->Nparametros;k++)
           printf("[%2.3f]\t",pEnj->Enj[i].Xi[k]);
    printf("\nV[%u]: ",i);
    for(k=0;k<pEnj->Nparametros;k++)
           printf("[%2.3f]\t",pEnj->Enj[i].Vi[k]);
    printf("\nP[%u]: ",i);
    for(k=0;k<pEnj->Nparametros;k++)
           printf("[%2.3f]\t",pEnj->Enj[i].Pi[k]);
    printf("\nXFit[%u]: [%2.3f]",i,pEnj->Enj[i].XFit);
    printf("\nPFit[%u]: [%2.3f]\n",i,pEnj->Enj[i].PFit);
}

void ShowEnjambre(ENJAMBRE* pEnj)
{
    unsigned int i;
    //PARA TODAS LAS PARTICULAS
    for(i=0;i<pEnj->Nparticulas;i++)
        ShowParticula(pEnj,i);
}

void EliminarEnjambre(ENJAMBRE* pEnj)
{
     unsigned int k;
     //LIBERAR MEMORIA PARA CADA PARTICULA
     for(k=0;k<pEnj->Nparticulas;k++)
    {
        free(pEnj->Enj[k].Xi);
        free(pEnj->Enj[k].Vi);
        free(pEnj->Enj[k].Pi);
    }
    //LIBERAR MEMORIA DE LAS PARTTICULAS
    free(pEnj->Enj);
    //LIBERAR MEMORIA DEL ENJAMBRE
    free(pEnj);
}

void EvaluarEnjambre(ENJAMBRE* pEnj)
{
    unsigned int k;
    //EVALUAR CADA PARTICULA
    for(k=0;k<pEnj->Nparticulas;k++)
        pEnj->Enj[k].XFit=50-((pEnj->Enj[k].Xi[0]-5)*(pEnj->Enj[k].Xi[0]-5)+((pEnj->Enj[k].Xi[1]-5)*(pEnj->Enj[k].Xi[1]-5)));
}

void InicializarMejores(ENJAMBRE* pEnj)
{
  unsigned int k;
  float Best;
  Best=pEnj->Enj[0].XFit;                     //BEST SE INICIALIZA CON EL VALOR FITNESS DE LA POSICION DE LA PRIMERA PARTICULA
  //PARA TODAS LAS PARTICULAS
    for(k=0;k<pEnj->Nparticulas;k++)
    {
        pEnj->Enj[k].PFit=pEnj->Enj[k].XFit;         //CADA PFit DE CADA PARTICULA SE INICIALIZA CON EL VALOR FITNESS DE LA POSICION DE ELLA MISMA
        if(pEnj->Enj[k].PFit>Best)                 //ENTRA SI EL PFit DE CADA PARTICULA ES MAYOR QUE BEST
        {
            pEnj->idGbest=k;            // SE HALLA LA K-ESIMA PARTICULA QUE SE CONSIDERA LA MEJOR
            Best=pEnj->Enj[k].PFit;  // SE AGINA EL VALOR PFit DE LA K-ESIMA PARTICULA A BEST
        }
    }
}

void ActualizarVelocidad(ENJAMBRE* pEnj)
{
    unsigned int i,k;
    float Y1,Y2,aux;
    //PARA TODAS LAS PARTICULAS
    for(i=0;i<pEnj->Nparticulas;i++)
         for(k=0;k<pEnj->Nparametros;k++)
        {
           Y1=((double)rand()/RAND_MAX);             //Numero de 0 a 1
           Y2=((double)rand()/RAND_MAX);             //Numero de 0 a 1
           aux=pEnj->Enj[i].Vi[k]+pEnj->C1*Y1*(pEnj->Enj[i].Pi[k]-pEnj->Enj[i].Xi[k])+
                    pEnj->C2*Y2*(pEnj->Enj[pEnj->idGbest].Pi[k]-pEnj->Enj[i].Xi[k]);
           if(aux>pEnj->Vmax)
             {pEnj->Enj[i].Vi[k]=pEnj->Vmax;
              break;}
           if(aux<pEnj->Vmin)
            {pEnj->Enj[i].Vi[k]=pEnj->Vmin;
             break;}
          pEnj->Enj[i].Vi[k]= aux;
        }
}

void ActualizarPosicion(ENJAMBRE* pEnj)
{
    unsigned int i,k;
    //PARA TODAS LAS PARTICULAS
    for(i=0;i<pEnj->Nparticulas;i++)
         for(k=0;k<pEnj->Nparametros;k++)
             pEnj->Enj[i].Xi[k]+=pEnj->Enj[i].Vi[k];
}

void ActualizarMejores(ENJAMBRE* pEnj)
{
    unsigned int i,k;
    float Best;
    Best=pEnj->Enj[pEnj->idGbest].PFit;             //SE ASIGNA A BEST EL PFit DE LA MEJOR PARTICULA
    for(i=0;i<pEnj->Nparticulas;i++)
    {
        if(pEnj->Enj[i].XFit>pEnj->Enj[i].PFit)       //ENTRA SI EL XFfit DE LA PARTICULA ES MAYOR QUE SU PFit, AS\CD PARA CADA PARTICULA
        {
            for(k=0;k<pEnj->Nparametros;k++)
            {
                 pEnj->Enj[i].Pi[k]=pEnj->Enj[i].Xi[k];   //SE ASIGNA LA POSICION DE LA PARTICULA A SU MEJOR POSICION
                 pEnj->Enj[i].PFit=pEnj->Enj[i].XFit;     //SE ASIGNA LA MEJOR POSICION A SU VALOR FITNESS DE LA MEJOR POSICION
            }
        }
        if(pEnj->Enj[i].PFit>Best)              //ENTRA SI EL PFit DE LA PARTICULA ES MAYOR A BEST, ASI PARA CADA PARTICULA
        {
            pEnj->idGbest=i;                      //LA i-ESIMA PARTICULA ES IDENTIFICADA COMO LA MEJOR GLOBAL
            Best= pEnj->Enj[i].PFit;           //EL VALOR FITNESS DE LA MEJOR POSICION SE ASIGNA A BEST
        }

    }

}
