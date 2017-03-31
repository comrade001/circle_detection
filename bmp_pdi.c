#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

typedef unsigned char byte;		// Tipo de dato de 1 byte
typedef unsigned short int word;	// Tipo de dato de 2 bytes
typedef unsigned long  int dword;	// Tipo de dato de 4 bytes

/*Sintonizacion del PSO*/
const unsigned int NUMEROdePARTICULAS=90;
const unsigned int NUMERO_PARAMETROS=6;
const unsigned int NUMERO_ITERACIONES=60;
const float LimiteInfVel=-1.0;
const float LimiteSupVel=1.0;

typedef struct{
	byte    id[2];  // Identificador de fila BMP
	word    offset; // Offset al principio de la imagen
	word    ancho;  // Columnas de la imagen
	word    alto;   // Filas de la imagen
       	byte    bpp;    // Bits de color por pixel
       	int     size;   // Tamaño de la imagen
	byte    *head;  // Puntero al encabezado
	float   *imx;   // Puntero al inicio de la imagen
}gcIMG;

/*Definicion de la estructura particula*/
typedef struct{
	float* Xi;    	//Posicion
        float* Vi;    	//Velocidad
        float* Pi;    	//Mejor posicion
        float XFit;  	//Valor fitness de la posicion
        float PFit;  	//Valor fitness de la mejor posicion
	int *x_c;	//Componente en X del circulo calculado	
	int *y_c;	//Componente en Y del circulo calculado
	int *rad;	//Radio del circulo calculado
}PARTICULA;

/*Definicion de la estructura enjambre*/
typedef struct{
	unsigned int Nparticulas;
        unsigned int Nparametros;
        unsigned int idGbest;	//id global best
        float Vmin;		//Velocidad minima
        float Vmax;		//Velocidad maxima
        float C1;		//Coeficiente de aceleracion 1
        float C2;		//Coeficiente de aceleracion 2
        PARTICULA* Enj;		//Puntero Enj a PARTICULA	
}ENJAMBRE;

/*Definicion de la estructura para pixeles activos*/
typedef struct{
	int *x;		//Posicion pixel activo
}ACT_PIX;

/*Prototipo de funciones (Imagen)*/
gcIMG* gcGetImgBmp(char *ruta);
void gcPutImgBmp(char *ruta, gcIMG *img);
gcIMG* gcNewImg(int ancho,int alto);
void gcFreeImg (gcIMG *img);

/*Prototipo de funciones (PSO)*/
ENJAMBRE* CrearEnjambre(const unsigned int Nparticulas,
		const unsigned int Nparametros);
void InicializarEnjambre(ENJAMBRE* pEnj, ACT_PIX *px, const int LInf, 
		const int LSup, const float C1, const float C2, 
		const float Vinf, const float Vsup);
void ShowParticula(ENJAMBRE* pEnj, const unsigned int i);
void ShowEnjambre(ENJAMBRE* pEnj);
void EliminarEnjambre(ENJAMBRE* pEnj);
void EvaluarEnjambre(ENJAMBRE* pEnj, gcIMG *img);
void InicializarMejores(ENJAMBRE* pEnj);
void ActualizarVelocidad(ENJAMBRE* pEnj);
void ActualizarPosicion(ENJAMBRE* pEnj);
void ActualizarMejores(ENJAMBRE* pEnj);

int main(void)
{ 
	gcIMG *Img1;		//Puntero Img1 a struct gcIMG
	ENJAMBRE *Ejemplo;	//Puntero Ejemplo a struct ENJAMBRE
	ACT_PIX *px;		//Puntero px a struct ACT_PIX
	unsigned int i, j, index = 0;
	unsigned int It=0,MaximoIteraciones=NUMERO_ITERACIONES;
	srand(time(NULL));	//Semilla a partir de reloj del sistema
	
	/*Abrir la imagen "input.bmp"*/
  	Img1 = gcGetImgBmp("input.bmp");

	/*Crear espacio en memoria para la struct px*/
	px = (ACT_PIX *)malloc(sizeof(ACT_PIX)); 	
	px -> x = (int *)malloc(sizeof(int)*Img1->size);
	
	/*Deteccion de pixeles activos*/
	for(i=0; i < Img1->ancho; i++)		//Recorrido por filas
		for(j=0; j < Img1->alto; j++)	//Recorrido por columnas
			if(Img1->imx[i*Img1->ancho+j] == 0) //Si es pixel activo
			{
				px->x[index]=i;
				px->x[index+1]=j;
				index+=2;
			}

	/*Creacion del enjambre*/
	Ejemplo = CrearEnjambre(NUMEROdePARTICULAS, NUMERO_PARAMETROS);
	/*Inicializacion del enjambre*/
	InicializarEnjambre(Ejemplo, px, 0, index, 2, 2, 
			LimiteInfVel, LimiteSupVel);
	/*Evaluacion del enjambre*/
	EvaluarEnjambre(Ejemplo, Img1);
	/*Inicializacion de los mejores*/
	InicializarMejores(Ejemplo);
	
	while(It<MaximoIteraciones)
	{
		/*Acualizacion de la velocidad de la particula*/
		ActualizarVelocidad(Ejemplo);
		/*Actualizacion de la posicion de la particula*/
		ActualizarPosicion(Ejemplo);
		/*Actualización de la mejor particula*/
		ActualizarMejores(Ejemplo);
		/*Evaluacion del enjambre*/
		EvaluarEnjambre(Ejemplo, Img1);
		It++;
		printf("\n--------------------------------\n");
		printf("\nIteracion %u: Best=%u\n", It, Ejemplo -> idGbest);
		printf("\n--------------------------------\n");	
		/*Muestra el enjambre en pantalla*/
		ShowEnjambre(Ejemplo);		
	}

	/*Liberacion de las particulas*/
	EliminarEnjambre(Ejemplo);

	/*Devolver espacio al HEAP*/
	free(px->x);
	free(px);
	return 0;
}

/*Funcion que crea espacio en memoria para todo el enjambre*/
ENJAMBRE* CrearEnjambre(const unsigned int Nparticulas,const unsigned int Nparametros)
{
	unsigned int k;
	ENJAMBRE* ptr;
    	/*Asignar memoria para la estructura enjambre*/
    	ptr=(ENJAMBRE*)malloc(sizeof(ENJAMBRE));
    	if(ptr==NULL)
    	{
        	printf("ERROR AL ASIGNAR MEMORIA A LA ESTRUCTURA ENJAMBRE");
        	exit(0);
    	}
    	/*Inicializar la estructura
     	* Asigna numero de particulas*/
    	ptr->Nparticulas=Nparticulas;
    	/*Asigna numero de parametros*/
    	ptr->Nparametros=Nparametros;
    	/*Asigna memoria para las particulas*/
	ptr->Enj=(PARTICULA*)malloc(sizeof(PARTICULA)*Nparticulas);
    	if(ptr->Enj==NULL)
    	{
    	    printf("ERROR AL ASIGNAR MEMORIA A LAS PARTICULAS");
    	    exit(0);
    	}
    	/*Asignacion de memoria para los miembros de la particula*/
    	for(k=0;k<Nparticulas;k++)
    	{
        	ptr->Enj[k].Xi=(float*)malloc(sizeof(float)*Nparametros);
        	ptr->Enj[k].Vi=(float*)malloc(sizeof(float)*Nparametros);
        	ptr->Enj[k].Pi=(float*)malloc(sizeof(float)*Nparametros);
		ptr->Enj[k].x_c=(int*)malloc(sizeof(int));
		ptr->Enj[k].y_c=(int*)malloc(sizeof(int));
		ptr->Enj[k].rad=(int*)malloc(sizeof(int));
    	}
    	return(ptr);
}

/*Funcion para la inicializacion del enjambre*/
void InicializarEnjambre(ENJAMBRE* pEnj, ACT_PIX *px, const int LInf, 
		const int LSup, const float C1, const float C2, 
		const float Vinf, const float Vsup)
{
	unsigned int i, k1, k2;
    	unsigned int random;
    	/*Para todas las particulas*/
    	for(i=0;i<pEnj->Nparticulas;i++)
	{
		/*Para todo los parametros*/
		for(k1=0, k2=1;k1<pEnj->Nparametros;k1+=2, k2+=2)
        	{
			/*Valor aleatorio entre [0, LSup]*/
           		random = (rand()%LSup+1);	
			/*Si random es impar significa que estamos
			 * en un valor de ordenada*/
			if((random & 1) == 1)
				random++;
			/*El valor de random se le da como primera posicion*/
           		pEnj->Enj[i].Xi[k1]=px->x[random];                                
			pEnj->Enj[i].Xi[k2]=px->x[random+1];
				
			/*EL valor de 0 se agrega porque suponemos que partimos
			 * del reposo*/
	   		pEnj->Enj[i].Vi[k1]=0;    
			pEnj->Enj[i].Vi[k2]=0;
           		
			/*El valor de random es la unica mejor posicion
			 * que se conoce*/
			pEnj->Enj[i].Pi[k1]=px->x[random];
			pEnj->Enj[i].Pi[k2]=px->x[random+1];
	   		/*El valor fitness de la posicion se inicializa en 0*/
			pEnj->Enj[i].XFit=0;
		}		
	}
			/*Inicializacion de las constantes C1 y C2
			 * y de los valores de frontera de Vmin y Vmax*/
        		pEnj->C1=C1;
        		pEnj->C2=C2;
        		pEnj->Vmin=Vinf;
        		pEnj->Vmax=Vsup;
}

/*Funcion para vizualizacion en pantalla de la particula*/
void ShowParticula(ENJAMBRE* pEnj, const unsigned int i)
{
	unsigned int k;
    	printf("\nX[%u]: ",i);
    	for(k=0;k<pEnj->Nparametros;k++)
    		printf("[%d] ",(int)pEnj->Enj[i].Xi[k]);
    	printf("\nV[%u]: ",i);
    	for(k=0;k<pEnj->Nparametros;k++)
		printf("[%d] ",(int)pEnj->Enj[i].Vi[k]);
    	printf("\nP[%u]: ",i);
    	for(k=0;k<pEnj->Nparametros;k++)
    		printf("[%d] ",(int)pEnj->Enj[i].Pi[k]);
    	printf("\nXFit[%u]: [%d]",i,(int)pEnj->Enj[i].XFit);
    	
	/*Si los votos sobrepasan el valor podemos decir
    	 * que se trata de un circulo en cuestion*/
    	if(pEnj->Enj[i].PFit > 180)
    		    printf("\nPFit[%u]: [%d]   <--------------(circulo)\n",i,(int)pEnj->Enj[i].PFit);
    	else
    	        printf("\nPFit[%u]: [%d]\n",i,(int)pEnj->Enj[i].PFit);

    	/*Si el calculo del centro del circulo sale de nuestro espacio
	 * cosideramos que el calculo de dicha particula esta perdido
	 * debido a colinealidades en los parametros de la particula*/    
    	if(*pEnj->Enj[i].x_c > 0 && *pEnj->Enj[i].y_c > 0)
    	{
    	        printf("x_c: [%d] y_c: [%d]\n", *pEnj->Enj[i].x_c, *pEnj->Enj[i].y_c);
    	        printf("radius: [%d]\n", *pEnj->Enj[i].rad);
    	}
    	else if(*pEnj->Enj[i].x_c < 0 && *pEnj->Enj[i].y_c > 0)
    	{
    	        printf("x_c: [%s] y_c: [%d]\n", "lost", *pEnj->Enj[i].y_c); 
    	        printf("radius: [%s]\n", "lost");
    	}
    	else if(*pEnj->Enj[i].x_c > 0 && *pEnj->Enj[i].y_c < 0)
    	{
    	        printf("x_c: [%d] y_c: [%s]\n", *pEnj->Enj[i].x_c, "lost");	
    	        printf("radius: [%s]\n", "lost");
    	}	
}

/*Funcion para la vizualiacion en pantalla de todo el enjambre*/
void ShowEnjambre(ENJAMBRE* pEnj)
{
    	unsigned int i;
    	/*Para todas las particulas*/
    	for(i=0;i<pEnj->Nparticulas;i++)
    		ShowParticula(pEnj,i);
}

/*Recuperar espacio en memoria usado por el enjambre y devuelto al HEAP*/
void EliminarEnjambre(ENJAMBRE* pEnj)
{
	unsigned int k;
     	/*Liberar memoria para cada particula*/
     	for(k=0;k<pEnj->Nparticulas;k++)
    	{
        	free(pEnj->Enj[k].Xi);
        	free(pEnj->Enj[k].Vi);
        	free(pEnj->Enj[k].Pi);
        	free(pEnj->Enj[k].x_c);
        	free(pEnj->Enj[k].y_c);
        	free(pEnj->Enj[k].rad);
    	}
    	/*Liberar memoria de las particulas*/
    	free(pEnj->Enj);
    	/*Liberar memoria del enjambre*/
    	free(pEnj);
}

/*Evaluacion de cada particula*/
void EvaluarEnjambre(ENJAMBRE* pEnj, gcIMG *img)
{
	unsigned int k;
    	int tetha;
    	int x, y;
    	/*Evaluacion de cada particula*/
    	for(k=0;k<pEnj->Nparticulas;k++)
    	{
		pEnj->Enj[k].XFit=0;
		/*Calculo del centro del circulo*/
		*pEnj->Enj[k].x_c = (int)(((pEnj->Enj[k].Xi[2]*pEnj->Enj[k].Xi[2]+pEnj->Enj[k].Xi[3]*pEnj->Enj[k].Xi[3]-(pEnj->Enj[k].Xi[0]*pEnj->Enj[k].Xi[0]+pEnj->Enj[k].Xi[1]*pEnj->Enj[k].Xi[1]))*(2*(pEnj->Enj[k].Xi[5]-pEnj->Enj[k].Xi[1])) - (2*(pEnj->Enj[k].Xi[3]-pEnj->Enj[k].Xi[1]))*(pEnj->Enj[k].Xi[4]*pEnj->Enj[k].Xi[4]+pEnj->Enj[k].Xi[5]*pEnj->Enj[k].Xi[5]-(pEnj->Enj[k].Xi[0]*pEnj->Enj[k].Xi[0]+pEnj->Enj[k].Xi[1]*pEnj->Enj[k].Xi[1]))) / (4*((pEnj->Enj[k].Xi[2]-pEnj->Enj[k].Xi[0])*(pEnj->Enj[k].Xi[5]-pEnj->Enj[k].Xi[1])-(pEnj->Enj[k].Xi[4]-pEnj->Enj[k].Xi[0])*(pEnj->Enj[k].Xi[3]-pEnj->Enj[k].Xi[1]))));
    		*pEnj->Enj[k].y_c = (int)((2*(pEnj->Enj[k].Xi[2]-pEnj->Enj[k].Xi[0])*(pEnj->Enj[k].Xi[4]*pEnj->Enj[k].Xi[4]+pEnj->Enj[k].Xi[5]*pEnj->Enj[k].Xi[5]-(pEnj->Enj[k].Xi[0]*pEnj->Enj[k].Xi[0]+pEnj->Enj[k].Xi[1]*pEnj->Enj[k].Xi[1])) - 2*(pEnj->Enj[k].Xi[4]-pEnj->Enj[k].Xi[0])*(pEnj->Enj[k].Xi[2]*pEnj->Enj[k].Xi[2]+pEnj->Enj[k].Xi[3]*pEnj->Enj[k].Xi[3]-(pEnj->Enj[k].Xi[0]*pEnj->Enj[k].Xi[0]+pEnj->Enj[k].Xi[1]*pEnj->Enj[k].Xi[1])))  / (4*((pEnj->Enj[k].Xi[2]-pEnj->Enj[k].Xi[0])*(pEnj->Enj[k].Xi[5]-pEnj->Enj[k].Xi[1])-(pEnj->Enj[k].Xi[4]-pEnj->Enj[k].Xi[0])*(pEnj->Enj[k].Xi[3]-pEnj->Enj[k].Xi[1]))));
		/*Calculo del radio del circulo*/
		*pEnj->Enj[k].rad = sqrt((pEnj->Enj[k].Xi[0]-*pEnj->Enj[k].x_c)*(pEnj->Enj[k].Xi[0]-*pEnj->Enj[k].x_c)+(pEnj->Enj[k].Xi[1]-*pEnj->Enj[k].y_c)*(pEnj->Enj[k].Xi[1]-*pEnj->Enj[k].y_c));
	
	/*Validacion de la existencia del circulo*/
	for(tetha=0; tetha<360; tetha++)
	{
		x=(int)(*pEnj->Enj[k].rad*cos(tetha*M_PI/180.0)+*pEnj->Enj[k].x_c);
		y=(int)(*pEnj->Enj[k].rad*sin(tetha*M_PI/180.0)+*pEnj->Enj[k].y_c);
		/*La componente X, Y debe estar dentro de los siguientes 
		 * parametros para evitar error de fragmentacion*/
		if(x < 256 && y < 256 && x > 0 && y > 0)
		{
			/*Si hay pixel*/
			if(img->imx[x*img->ancho+y] == 0)
				/*Se adiciona un voto*/
				pEnj->Enj[k].XFit++;	
		}
	}

    }
}

/*Inicializacion de los mejores*/
void InicializarMejores(ENJAMBRE* pEnj)
{
  	unsigned int k;
  	float Best;
  	pEnj->idGbest=0;
  	/*Best se inicializa con el valor fitnees de la posicion
  	 * de la primera particula*/
  	Best=pEnj->Enj[0].XFit;

	/*Para cada particula*/
    	for(k=0;k<pEnj->Nparticulas;k++)
    	{
		/*Cada PFit de cada particula se inicializa con el valor 
		 * fitness de la posicion de la misma*/
    		pEnj->Enj[k].PFit=pEnj->Enj[k].XFit;  
		/*Entra si el PFit de cada particula es mayor que best*/
    	    	if(pEnj->Enj[k].PFit>Best)              
    	    	{
			/*Se halla la k-esima particula que se considera la mejor*/
    	        	pEnj->idGbest=k;           
			/*Se asigna el valor PFit de la k-esima particula a best*/
    	        	Best=pEnj->Enj[k].PFit;  
    	    	}
    	}
}

/*Actualizar velocidad de las particulas*/
void ActualizarVelocidad(ENJAMBRE* pEnj)
{
	unsigned int i,k=0;
    	float Y1,Y2,aux;
    	/*Para todas las particulas*/
    	for(i=0;i<pEnj->Nparticulas;i++)
	/*Para todos los parametros*/
        	for(k=0;k<pEnj->Nparametros;k++)
        	{
        		Y1=((double)rand()/RAND_MAX);	//Numero de 0 a 1
           		Y2=((double)rand()/RAND_MAX);	//Numero de 0 a 1
           		aux=pEnj->Enj[i].Vi[k]+pEnj->C1*Y1*(pEnj->Enj[i].Pi[k]-pEnj->Enj[i].Xi[k])+
                    	pEnj->C2*Y2*(pEnj->Enj[pEnj->idGbest].Pi[k]-pEnj->Enj[i].Xi[k]);
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
			pEnj->Enj[i].Vi[k]= aux;
        	}
}

/*Actualizacion de la posicion de la particula*/
void ActualizarPosicion(ENJAMBRE* pEnj)
{
    	unsigned int i,k;
    	/*Para las particulas*/
    	for(i=0;i<pEnj->Nparticulas;i++)
		/*Para todos los parametros*/
        	for(k=0;k<pEnj->Nparametros;k++)
             		pEnj->Enj[i].Xi[k]+=pEnj->Enj[i].Vi[k];
}

/*Actualizacion de las mejores particulas*/
void ActualizarMejores(ENJAMBRE* pEnj)
{
    	unsigned int i,k;
    	float Best;
	/*Se asigna a best el PFit de la mejor particula*/
    	Best=pEnj->Enj[pEnj->idGbest].PFit;            
    	for(i=0;i<pEnj->Nparticulas;i++)
    	{
		/*Entra si el XFit de la particula es mayor que su PFit*/
    	    	if(pEnj->Enj[i].XFit>pEnj->Enj[i].PFit)       
    	    	{
			/*Para todos los parametros*/
    	        	for(k=0;k<pEnj->Nparametros;k++)
    	        	{
				/*Se asigna la posicion de la particula a su 
				 * mejor posicion*/
    	             		pEnj->Enj[i].Pi[k]=pEnj->Enj[i].Xi[k];   
				/*Se asigna la mejor posicion a su valor fitness 
				 * de la mejor posicion*/
    	             		pEnj->Enj[i].PFit=pEnj->Enj[i].XFit;     
    	        	}
    	    	}
		/*Entra si el PFit de la partciula es mayor a Best*/ 
    	    	if(pEnj->Enj[i].PFit>Best)             
    	    	{
			/*La i-esima particula es identificada*/
    	        	pEnj->idGbest=i;                      
			/*El valor fitness de la mejor posicion se asigna a best*/
    	        	Best= pEnj->Enj[i].PFit;          
    	    	}
	}
}

gcIMG* gcGetImgBmp(char *ruta)
{ 
	gcIMG *img;
	FILE *file;
	int  i,j,a,ar;

	// Abrir Archivo de entrada
	if((file = fopen(ruta,"rb"))==NULL )
      	{ 
		printf(" Error al Abrir Archivo \n");
		exit(1);
	}
	// Asignar memoria para la estructura gcIMG
  	if((img = (gcIMG *) calloc(1,sizeof(gcIMG)) ) == NULL)
      	{ 
		printf("Error al reservar memoria para gcIMG \n");
 	     	exit (1);
	}

  	fread(img->id,2,1,file);      // Lee 2 bytes del identificador
	fseek(file,10,SEEK_SET);      // Se posiciona en Data offset
  	fread(&img->offset,2,1,file); // Lee el offset de la Imagen
	fseek(file,18,SEEK_SET);      // Se posiciona en Width
	fread(&img->ancho,2,1,file);  // Lee el ancho de la Imagen
	fseek(file,22,SEEK_SET);      // Se posiciona en Height
	fread(&img->alto,2,1,file);   // Lee el alto de la Imagen
	fseek(file,28,SEEK_SET);      // Se posiciona en Bits p/pixel
	fread(&img->bpp,1,1,file);    // Lee los Bpp
	fseek(file,34,SEEK_SET);      // Se posiciona en Size
	fread(&img->size,4,1,file);   // Lee el tamaño de la Imagen */

	// Comprobar archivo valido
  	if((img->id[0]!='B')||(img->id[1]!='M') )
      	{ 
		printf("Archivo de Formato No Valido \n");
	        exit (1);
        }

	// Asignar memoria para el encabezado
  	if((img->head = (unsigned char *) malloc(img->offset)) == NULL )
     	{ 
		printf("Error al reservar memoria para el encabezado \n");
	       exit (1);
       	}

	// Asignar memoria para la imagen real
	if((img->imx =(float *)calloc(img->ancho*img->alto,sizeof(float))) == NULL )
	{ 
		printf("Error al reservar memoria para la imagen \n");
       		exit (1);
	}

	// Lectura del encabezado
	rewind(file);
	fread(img->head,1078,1,file);

	// Lectura de la imagen
	a=img->ancho;
	ar=img->size/img->alto;               //calcula el ancho real
	fseek(file,img->offset,SEEK_SET);     // Se posiciona al inicio de la imagen
	for (i=0; i<img->alto; i++)
	{ 
		for(j=0; j<img->ancho; j++)
		img->imx[i*a+j]=(float)fgetc(file);
		if(ar!=a) for(j=0;j<ar-a;j++) fgetc(file);  // Si el ancho es mayor
      	}                                           // brinca esos datos
	fclose(file);
	img->size=img->ancho*img->alto;       //Asigna el Tamaño Real de la Imagen
	return img;
}

void gcPutImgBmp(char *ruta, gcIMG *img)
{ 
	FILE *file;
	int aux,zero=0,i,j,offset,Newancho;

	// Crear un Archivo nuevo
	if((file = fopen(ruta,"w+b")) == NULL)
        { 
		printf("\nError abriendo el archivo \n");
		exit(1);
        }
	//Checar si el ancho es multiplo de 4
	offset=img->ancho%4;
	if(offset)
		Newancho=img->ancho+(4-offset); //Si no hacerlo multiplo
	else
		Newancho=img->ancho;           // Si, si mantenerlo

	// Checar el encabezado
	if (img->head) 
	{ 
		img->size=(Newancho*img->alto); //Modificar el bitmap size
                fwrite(img->head,1078,1,file);
	}
	// Generar encabezado:
     	else 
	{
		fputc('B',file); fputc('M',file);   // Escribe BMP Identificador
	    	aux = Newancho * img->alto + 1078;
	    	fwrite(&aux,4,1,file);              // Escribe File Size
	    	fwrite(&zero,4,1,file);             // Escribe Word Reserved
            	aux=1078;
	    	fwrite(&aux,4,1,file);              // Escribe Data Offset
	    	// Image Header
		aux=40;
	    	fwrite(&aux,4,1,file);              // Escribe Header Size
	    	aux=img->ancho;
	    	fwrite(&aux,4,1,file);              // Escribe Width
	    	aux=img->alto;
	    	fwrite(&aux,4,1,file);              // Escribe Height
	    	aux=1;
	    	fwrite(&aux,2,1,file);              // Escribe Planes
	    	aux=8;
	    	fwrite(&aux,2,1,file);              // Escribe Bits p/pixel
	    	aux=0;
	    	fwrite(&aux,4,1,file);              // Escribe Compression
            	aux=(Newancho*img->alto);
	    	fwrite(&aux,4,1,file);              // Escribe Bitmap Size
	    	aux=0;
	    	fwrite(&aux,4,1,file);              // Escribe HResolution
            	fwrite(&aux,4,1,file);              // Escribe VResolution
            	aux=256;
            	fwrite(&aux,4,1,file);              // Escirbe Colors used
            	aux=0;
            	fwrite(&aux,4,1,file);              // Escirbe Important Colors

		// Escritura de la paleta
   		for (aux=0; aux<256; aux++)
                { 
			for (i=0; i<3; i++) fwrite(&aux,1,1,file);
			fwrite(&zero,1,1,file);
		}
	}
	// Escritura del mapa de bits
	aux=img->ancho;
	for(i=0;i<img->alto;i++)
      		for(j=0;j<Newancho;j++)
        	{ 
			if(j>aux-1)
				fputc(0,file);
		        else 
				fputc((unsigned char)img->imx[i*aux+j],file);
		}
  	fclose(file);
}

gcIMG *gcNewImg(int ancho,int alto)
{
	gcIMG *img;
	//int i;

  	if (( img = (gcIMG *) calloc(1,sizeof(gcIMG)) ) == NULL)
        { 
		printf("Error al reservar memoria para gcIMG\n");
          	exit (1);
        }
  	img->ancho = ancho;
	img->alto  = alto;
  	img->size  = ancho*alto;
  	if (( img->imx = (float *) calloc(img->size,sizeof(float)) ) == NULL)
        { 
		printf("Error al reservar memoria para la Imagen \n");
          	exit (1);
        }
  	img->head = NULL;
  	return img;
}

void gcFreeImg(gcIMG *img)
{ 
	free(img->head);
   	free(img->imx);
   	free(img);
}		
