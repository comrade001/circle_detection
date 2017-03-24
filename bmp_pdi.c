#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
/* ***************** ESTRUCTURAS DE DATOS ****************************** */
typedef unsigned char byte;    		// Tipo de dato de 1 byte
typedef unsigned short int word;    	// Tipo de dato de 2 bytes
typedef unsigned long  int dword;   	// Tipo de dato de 4 bytes

//Sintonizacion del PSO
const unsigned int NUMEROdePARTICULAS=90;
const unsigned int NUMERO_PARAMETROS=6;
const unsigned int NUMERO_ITERACIONES=60;
//const float LimiteInf=0;
//const float LimiteSup=10;
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

//DEFINICION DE LA ESTRUCTURA PARTICULA
typedef struct{
                       float* Xi;    	//POSICION
                       float* Vi;    	//VELOCIDAD
                       float* Pi;    	//MEJOR POSICION
                       float XFit;  	//VALOR DE FITNESS DE LA POSICION
                       float PFit;  	//VALOR DE FITNESS DE LA MEJOR POSICION	
		       int *x_c;
		       int *y_c;
		       int *rad;
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

typedef struct{
	int *x;		//POSICION PIXEL ACTIVO
}ACT_PIX;

gcIMG* gcGetImgBmp(char *ruta);
void gcPutImgBmp(char *ruta, gcIMG *img);
gcIMG* gcNewImg(int ancho,int alto);
void gcFreeImg (gcIMG *img);

ENJAMBRE* CrearEnjambre(const unsigned int Nparticulas,const unsigned int Nparametros );
void InicializarEnjambre(ENJAMBRE* pEnj, ACT_PIX *px, const int LInf, const int LSup, const float C1, const float C2, const float Vinf, const float Vsup);
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
	gcIMG *Img1;
	ENJAMBRE *Ejemplo;
	ACT_PIX *px;
	unsigned int i, j, index = 0, pix_detect = 0;
	unsigned int It=0,MaximoIteraciones=NUMERO_ITERACIONES;
	srand(time(NULL));
	//Abrir la imagen input.bmp
  	Img1 = gcGetImgBmp("input.bmp");
	px = (ACT_PIX *)malloc(sizeof(ACT_PIX));
	px -> x = (int *)malloc(sizeof(int)*Img1->size);
	for(i=0; i < Img1->ancho; i++)
		for(j=0; j < Img1->alto; j++)
			//Si es un pixel activo
			if(Img1->imx[i*Img1->ancho+j] == 0)
			{
				px->x[index]=i;
				px->x[index+1]=j;
				//printf("%d, %d\n", px->x[index], px->x[index+1]);
				index+=2;
				pix_detect++;
			}

	Ejemplo = CrearEnjambre(NUMEROdePARTICULAS, NUMERO_PARAMETROS);
	InicializarEnjambre(Ejemplo, px, 0, index, 2, 2, LimiteInfVel, LimiteSupVel);
	EvaluarEnjambre(Ejemplo, Img1);
	InicializarMejores(Ejemplo);
	
	while(It<MaximoIteraciones)
	{
		ActualizarVelocidad(Ejemplo);
		ActualizarPosicion(Ejemplo);
		ActualizarMejores(Ejemplo);
		EvaluarEnjambre(Ejemplo, Img1);
		It++;
		printf("\n--------------------------------\n");
		printf("\nIteracion %u: Best=%u\n", It, Ejemplo -> idGbest);
		printf("\n--------------------------------\n");	
		ShowEnjambre(Ejemplo);		
	}

	//if(Ejemplo->Enj[Ejemplo->idGbest].PFit > 359)
	//	printf("Hay un circulo\n");
	EliminarEnjambre(Ejemplo);

	free(px->x);
	free(px);
	return 0;
}

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
	ptr->Enj[k].x_c=(int*)malloc(sizeof(int));
	ptr->Enj[k].y_c=(int*)malloc(sizeof(int));
	ptr->Enj[k].rad=(int*)malloc(sizeof(int));
    }
    return(ptr);
}

void InicializarEnjambre(ENJAMBRE* pEnj, ACT_PIX *px, const int LInf, const int LSup, const float C1, const float C2, const float Vinf, const float Vsup)
{
	unsigned int i, k1, k2;
    	unsigned int random;
    	//PARA TODAS LAS PARTICULAS
    	for(i=0;i<pEnj->Nparticulas;i++)
	{
		for(k1=0, k2=1;k1<pEnj->Nparametros;k1+=2, k2+=2)
        	{
           		random = (rand()%LSup+1);				//Valor aleatorio entre (0, LSup)
			if((random & 1) == 1)
				random++;
           		pEnj->Enj[i].Xi[k1]=px->x[random];                                // El valor de r se le da como primera posicion
			pEnj->Enj[i].Xi[k2]=px->x[random+1];
	   		pEnj->Enj[i].Vi[k1]=0;                               // El valor de 0 se agrega porque suponemos que no tenemos velocidad
			pEnj->Enj[i].Vi[k2]=0;
           		pEnj->Enj[i].Pi[k1]=px->x[random];                                // El valor de r es la unica mejor posicion que se conoce
			pEnj->Enj[i].Pi[k2]=px->x[random+1];
	   		pEnj->Enj[i].XFit=0;
		}		
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
            printf("[%d] ",(int)pEnj->Enj[i].Xi[k]);
    printf("\nV[%u]: ",i);
    for(k=0;k<pEnj->Nparametros;k++)
            printf("[%d] ",(int)pEnj->Enj[i].Vi[k]);
    printf("\nP[%u]: ",i);
    for(k=0;k<pEnj->Nparametros;k++)
            printf("[%d] ",(int)pEnj->Enj[i].Pi[k]);
    printf("\nXFit[%u]: [%d]",i,(int)pEnj->Enj[i].XFit);
    if(pEnj->Enj[i].PFit > 180)
    	    printf("\nPFit[%u]: [%d]   <--------------(circulo)\n",i,(int)pEnj->Enj[i].PFit);
    else
	    printf("\nPFit[%u]: [%d]\n",i,(int)pEnj->Enj[i].PFit);

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
        free(pEnj->Enj[k].x_c);
        free(pEnj->Enj[k].y_c);
        free(pEnj->Enj[k].rad);
    }
    //LIBERAR MEMORIA DE LAS PARTTICULAS
    free(pEnj->Enj);
    //LIBERAR MEMORIA DEL ENJAMBRE
    free(pEnj);
}

void EvaluarEnjambre(ENJAMBRE* pEnj, gcIMG *img)
{
    unsigned int k;
    int tetha;
    int x, y;
    //EVALUAR CADA PARTICULA
    for(k=0;k<pEnj->Nparticulas;k++)
    {
	pEnj->Enj[k].XFit=0;
        //pEnj->Enj[k].XFit=50-((pEnj->Enj[k].Xi[0]-5)*(pEnj->Enj[k].Xi[0]-5)+((pEnj->Enj[k].Xi[1]-5)*(pEnj->Enj[k].Xi[1]-5)));
	*pEnj->Enj[k].x_c = (int)(((pEnj->Enj[k].Xi[2]*pEnj->Enj[k].Xi[2]+pEnj->Enj[k].Xi[3]*pEnj->Enj[k].Xi[3]-(pEnj->Enj[k].Xi[0]*pEnj->Enj[k].Xi[0]+pEnj->Enj[k].Xi[1]*pEnj->Enj[k].Xi[1]))*(2*(pEnj->Enj[k].Xi[5]-pEnj->Enj[k].Xi[1])) - (2*(pEnj->Enj[k].Xi[3]-pEnj->Enj[k].Xi[1]))*(pEnj->Enj[k].Xi[4]*pEnj->Enj[k].Xi[4]+pEnj->Enj[k].Xi[5]*pEnj->Enj[k].Xi[5]-(pEnj->Enj[k].Xi[0]*pEnj->Enj[k].Xi[0]+pEnj->Enj[k].Xi[1]*pEnj->Enj[k].Xi[1]))) / (4*((pEnj->Enj[k].Xi[2]-pEnj->Enj[k].Xi[0])*(pEnj->Enj[k].Xi[5]-pEnj->Enj[k].Xi[1])-(pEnj->Enj[k].Xi[4]-pEnj->Enj[k].Xi[0])*(pEnj->Enj[k].Xi[3]-pEnj->Enj[k].Xi[1]))));
    	*pEnj->Enj[k].y_c = (int)((2*(pEnj->Enj[k].Xi[2]-pEnj->Enj[k].Xi[0])*(pEnj->Enj[k].Xi[4]*pEnj->Enj[k].Xi[4]+pEnj->Enj[k].Xi[5]*pEnj->Enj[k].Xi[5]-(pEnj->Enj[k].Xi[0]*pEnj->Enj[k].Xi[0]+pEnj->Enj[k].Xi[1]*pEnj->Enj[k].Xi[1])) - 2*(pEnj->Enj[k].Xi[4]-pEnj->Enj[k].Xi[0])*(pEnj->Enj[k].Xi[2]*pEnj->Enj[k].Xi[2]+pEnj->Enj[k].Xi[3]*pEnj->Enj[k].Xi[3]-(pEnj->Enj[k].Xi[0]*pEnj->Enj[k].Xi[0]+pEnj->Enj[k].Xi[1]*pEnj->Enj[k].Xi[1])))  / (4*((pEnj->Enj[k].Xi[2]-pEnj->Enj[k].Xi[0])*(pEnj->Enj[k].Xi[5]-pEnj->Enj[k].Xi[1])-(pEnj->Enj[k].Xi[4]-pEnj->Enj[k].Xi[0])*(pEnj->Enj[k].Xi[3]-pEnj->Enj[k].Xi[1]))));
	
	//if(*pEnj->Enj[k].x_c < 0 || *pEnj->Enj[k].y_c < 0)
	//{
	//	printf("\n*************************");
	//	printf("\nParticula perdida\n");
	//	printf("*************************\n");
	//	break;
	//}
	*pEnj->Enj[k].rad = sqrt((pEnj->Enj[k].Xi[0]-*pEnj->Enj[k].x_c)*(pEnj->Enj[k].Xi[0]-*pEnj->Enj[k].x_c)+(pEnj->Enj[k].Xi[1]-*pEnj->Enj[k].y_c)*(pEnj->Enj[k].Xi[1]-*pEnj->Enj[k].y_c));
	//pEnj->Enj[k].XFit = 0;

	//printf("x_c=%d, y_c=%d, rad=%d\n", x_c, y_c, rad);
	for(tetha=0; tetha<360; tetha++)
	{
		x=(int)(*pEnj->Enj[k].rad*cos(tetha*M_PI/180.0)+*pEnj->Enj[k].x_c);
		y=(int)(*pEnj->Enj[k].rad*sin(tetha*M_PI/180.0)+*pEnj->Enj[k].y_c);
		//printf("x,y=%d,%d\n", x, y);
		if(x < 256 && y < 256 && x > 0 && y > 0)
		{
			if(img->imx[x*img->ancho+y] == 0)
				pEnj->Enj[k].XFit++;	
		}
	}

    }
}

void InicializarMejores(ENJAMBRE* pEnj)
{
  unsigned int k;
  float Best;
  pEnj->idGbest=0;
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
    unsigned int i,k=0;
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
