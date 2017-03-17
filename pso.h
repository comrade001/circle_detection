#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*Problema de ejemplo:
 * Maximizar la funcion:
 * f(x,y) = 50-(x-5)^2-(y-5)^2;
 */

//Definicion de la estructura particula
typedef struct {
	float *Xi; 	//Posicion
	float *Vi;	//Velocidad
	float *Pi;	//Mejor posicion
	float XFit;	//Valor de Fitness de la Posicion
	float PFit;	//Valor de Fitness de la Mejor Posicion
}PARTICULA;

//Definicion de la estructura para el enjambre
typedef struct {
	unsigned int Nparticulas; 	//Numero de particulas
	unsigned int Nparametros;
	unsigned int idGbest;
	float Vmin;
	float Vmax;
	float C1;
	float C2;
	PARTICULA *Enj;
}ENJAMBRE;

//Definicion de constantes globales
const unsigned int NUMEROdePARTICULAS=100;
const unsigned int NUMEROdePARAMETROS=2;
const float LimiteInf=0;
const float LimiteSup=10;
const unsigned int NUMERO_ITERACIONES = 300;
const float LimiteInfVel=-1.0;
const float LimiteSupVel=1.0;

//Protocolo de funciones
ENJAMBRE *CrearEnjambre(const unsigned int Nparticulas, const unsigned int Nparametros);
void InicializarEnjambre(ENJAMBRE *pEnj, const float LInf, const float LSup, const float C1, const float C2, const float Vinf, const float Vsup);
void ShowParticula(ENJAMBRE *pEnj, const unsigned int i);
void ShowEnjambre(ENJAMBRE *pEnj);
void EliminarEnjambre(ENJAMBRE *pEnj);
void EvaluarEnjambre(ENJAMBRE *pEnj);
void ShowEvaluacion(ENJAMBRE *pEnj);
void InicializarMejores(ENJAMBRE *pEnj);
void ActualizarVelocidad(ENJAMBRE *pEnj);
void ActualizarPosicion(ENJAMBRE *pEnj);
void ActualizarMejores(ENJAMBRE *pEnj);

void ActualizarMejores(ENJAMBRE *pEnj)
{
	unsigned int i, k;
	float Best;
	Best = pEnj->Enj[pEnj->idGbest].PFit;
	for(i=0; i<pEnj->Nparticulas; i++)
	{
		if(pEnj->Enj[i].XFit > pEnj->Enj[i].PFit)
		{
			for(k=0; k<pEnj->Nparametros; k++)
			{
				pEnj->Enj[i].Pi[k]=pEnj->Enj[i].Xi[k];
				pEnj->Enj[i].PFit=pEnj->Enj[i].XFit;
			}
		}
		if(pEnj->Enj[i].PFit>Best)
		{
			pEnj->idGbest=i;
			Best = pEnj->Enj[i].PFit;
		}
	}
}

void ActualizarPosicion(ENJAMBRE *pEnj)
{
	unsigned int i, k;
	//Para todas las particulas
	for(i=0; i<pEnj->Nparticulas; i++)
		//Para todos los parametros
		for(k=0; k<pEnj->Nparametros; k++)
			pEnj->Enj[i].Xi[k]+=pEnj->Enj[i].Vi[k];

}

void ActualizarVelocidad(ENJAMBRE *pEnj)
{
	unsigned int i, k;
	float Y1, Y2, aux;

	//Para todas las particulas
	for(i=0; i<pEnj->Nparticulas; i++)
		//Para todos los parametros
		for(k=0; k<pEnj->Nparametros; k++)
		{
			Y1=((double)rand()/RAND_MAX);
			Y2=((double)rand()/RAND_MAX);
			//pEnj->Enj[i].Vi[k]+=pEnj->C1*Y1*(pEnj->Enj[i].Pi[k] - pEnj->Enj[i].Xi[k])+
				//pEnj->C2*Y2*(pEnj->Enj[pEnj->idGbest].Pi[k] - pEnj->Enj[i].Xi[k]);
			aux=pEnj->Enj[i].Vi[k]+pEnj->C1*Y1*(pEnj->Enj[i].Pi[k] - pEnj->Enj[i].Xi[k])+
				pEnj->C2*Y2*(pEnj->Enj[pEnj->idGbest].Pi[k] - pEnj->Enj[i].Xi[k]);
			if(aux>pEnj->Vmax)
			{
				pEnj->Enj[i].Vi[k]=pEnj->Vmax;
				break;
			}
			if(aux<pEnj->Vmin)
			{
				pEnj->Enj[i].Vi[k]=pEnj->Vmin;
				break;
			}
			pEnj->Enj[i].Vi[k]=aux;
		}
}


void InicializarMejores(ENJAMBRE *pEnj)
{
	unsigned int k;
	float Best;
	Best =  pEnj -> Enj[0].XFit;
	//Para todas las particulas
	for(k=0; k<pEnj->Nparticulas; k++)
	{
		pEnj -> Enj[k].PFit = pEnj -> Enj[k].XFit;
		if(pEnj -> Enj[k].PFit > Best)
		{
			pEnj -> idGbest = k;
			Best = pEnj->Enj[k].PFit;
		}
	}
}

void ShowEvaluacion(ENJAMBRE *pEnj)
{
	//Imprimir evaluacion
	unsigned int k;
	printf("\nEvaluacion\n");
	for(k=0; k < pEnj -> Nparticulas; k++)
		printf("XFit[%u]: %2.4f\n", k, pEnj->Enj[k].XFit);
}

void EvaluarEnjambre(ENJAMBRE *pEnj)
{
	unsigned int k;
	//Evaluar cada Particula
	for(k=0; k < pEnj -> Nparticulas; k++)
		pEnj -> Enj[k].XFit = 50 - ((pEnj->Enj[k].Xi[0]-5)*(pEnj->Enj[k].Xi[0]-5)+
			(pEnj->Enj[k].Xi[1]-5)*(pEnj->Enj[k].Xi[1]-5));
}

void EliminarEnjambre(ENJAMBRE *pEnj)
{
	unsigned int k;
	//Liberar memoria para todos los parametros de cada particula
	for(k=0; k < pEnj -> Nparticulas; k++)
	{
		free(pEnj -> Enj[k].Xi);
		free(pEnj -> Enj[k].Vi);
		free(pEnj -> Enj[k].Pi);
	}
	//Liberar memoria de las particulas
	free(pEnj -> Enj);
	//Liberar memoria del enjambre
	free(pEnj);
}

void ShowEnjambre(ENJAMBRE *pEnj)
{
	unsigned int i;
	//Para todas las particulas
	for(i = 0; i < pEnj -> Nparticulas; i++)
		ShowParticula(pEnj, i);
}

void ShowParticula(ENJAMBRE *pEnj, const unsigned int i)
{
	unsigned int k;
	printf("\nX%u ", i);
	for(k=0; k < pEnj -> Nparametros; k++)
		printf("%2.4f\t", pEnj->Enj[i].Xi[k]);
	
	printf("\nV%u ", i);
	for(k=0; k < pEnj -> Nparametros; k++)
		printf("%2.4f\t", pEnj->Enj[i].Vi[k]);
	
	printf("\nP%u ", i);
	for(k=0; k < pEnj -> Nparametros; k++)
		printf("%2.4f\t", pEnj->Enj[i].Pi[k]);
	printf("\nx%uFit: %2.4f", i, pEnj -> Enj[i].XFit);
	printf("\np%uFit: %2.4f", i, pEnj -> Enj[i].PFit);
	printf("\n");

}
void InicializarEnjambre(ENJAMBRE *pEnj, const float LInf, const float LSup, const float C1, const float C2, const float Vinf, const float Vsup)
{
	unsigned int i, k;
	float r;
	//Para todas las particulas 
	for(i = 0; i < pEnj -> Nparticulas; i++)
		for(k = 0; k < pEnj -> Nparametros; k++)
		{
			r = ((double)rand()/RAND_MAX);
			r = (LSup-LInf)*r+LInf;
			pEnj -> Enj[i].Xi[k] = r;
			pEnj -> Enj[i].Vi[k] = 0;
			pEnj -> Enj[i].Pi[k] = r;
		}
	pEnj->C1=C1;
	pEnj->C2=C2;
	pEnj->Vmax=Vsup;
	pEnj->Vmin=Vinf;
}

ENJAMBRE *CrearEnjambre(const unsigned int Nparticulas, const unsigned int Nparametros)
{
	ENJAMBRE *ptr;
	
	//Asignar memoria a la estructura enjambre
	ptr = (ENJAMBRE *)malloc(sizeof(ENJAMBRE));
	if(ptr == NULL)
	{
		printf("Error al asignar memoria a la estructura Enjambre");
		exit(0);
	}
	//Inicializacion de la estructura
	//Asigna numero de particlas
	ptr -> Nparticulas = Nparticulas;
	//Asigna numero de parametros
	ptr -> Nparametros = Nparametros;
	//Asigna memoria para las particulas
	ptr -> Enj =(PARTICULA *)malloc(sizeof(PARTICULA)*Nparticulas);
	if(ptr -> Enj == NULL)
	{
		printf("Error al asignar memoria a las Particulas");
		exit(0);
	}
	
	//Asignar memoria a cada particula
	unsigned int k;
	for(k=0; k < Nparticulas; k++)
	{
		ptr -> Enj[k].Xi=(float *)malloc(sizeof(float)*Nparametros);
		ptr -> Enj[k].Vi=(float *)malloc(sizeof(float)*Nparametros);
		ptr -> Enj[k].Pi=(float *)malloc(sizeof(float)*Nparametros);
	}
	return ptr;
}

