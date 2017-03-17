#include<stdio.h>
#include<stdlib.h>
#include"pso.h"
#include<math.h>
/* ***************** ESTRUCTURAS DE DATOS ****************************** */
typedef unsigned char byte;    		// Tipo de dato de 1 byte
typedef unsigned short int word;    	// Tipo de dato de 2 bytes
typedef unsigned long  int dword;   	// Tipo de dato de 4 bytes

//Sintonizacion del PSO
const unsigned int NUMEROdePARTICULAS=100;
const unsigned int NUMERO_PARAMETROS=2;
const unsigned int NUMERO_ITERACIONES=300;
const float LimiteInf=0;
const float LimiteSup=10;
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

typedef struct{
	int *x; 
	int *y;
	int *r;
}VOTES;

gcIMG* gcGetImgBmp(char *ruta);
void gcPutImgBmp(char *ruta, gcIMG *img);
gcIMG* gcNewImg(int ancho,int alto);
void gcFreeImg (gcIMG *img);

int main(void)
{ 
	//Declarar un Puntero a imagen
	gcIMG *Img1, *aux;
  	unsigned int i,j;
	unsigned int index=0;
	//Abrir una imagen llamada input.bmp
  	Img1=gcGetImgBmp("input.bmp");
	VOTES *vt;
 	vt = (VOTES *)malloc(sizeof(VOTES));
	vt -> x = (int *)malloc(sizeof(int)*Img1->size);
	vt -> y = (int *)malloc(sizeof(int)*Img1->size);
	//vt -> r = (int *)malloc(sizeof(int)*Img1->size);
  	

	aux=gcNewImg(Img1->ancho, Img1->alto);
	
	//Extraer los pixeles de la imagen de entrada
	for(i = 0; i < Img1->ancho; i++)
		for(j = 0; j < Img1->alto; j++)
			if(Img1->imx[i*Img1->ancho+j] == 0)
			{
				vt->x[index]=i;
				vt->y[index]=j;
				index++;
			}

	//for(i = 0; i < index; i++)
	//	printf("%d, %d\n", vt->x[i], vt->y[i]);
	//printf("%d pixeles detectados\r\n", index);
	
	
	//gcPutImgBmp("Ejemplo.bmp", Img1);
	//Libera la Imagen utilizada
	gcFreeImg(Img1);
	gcFreeImg(aux);
	
	free(vt->x);
	free(vt->y);
	free(vt->r);
	free(vt);
	vt->x = NULL;
	vt->y = NULL;
	vt->r = NULL;
	vt = NULL;

	/*Prueba Pso.h*/
	ENJAMBRE* Ejemplo;    //CREAR MEMORIA PARA ENJAMBRE
    	unsigned int It=0,MaximoIteraciones=NUMERO_ITERACIONES;
    	Ejemplo=CrearEnjambre(NUMEROdePARTICULAS,NUMERO_PARAMETROS);
	InicializarEnjambre(Ejemplo,0,Img1->ancho/2,2,2,LimiteInfVel,LimiteSupVel); //INICIALIZAR POSICIONES DE LAS PARTICULAS ENTRE LOS LIMITES DEL ESPACIO DE BUSQUEDA DEL PROBLEMA
    	EvaluarEnjambre(Ejemplo); //EVALUAR EL FITNESS DE CADA PARTICULA
    	InicializarMejores(Ejemplo); //INICIALIZAR EL FITNESS DE LA MEJOR POSICION DE CADA PARTICULA E IDENTIFICAR EL INDICE DE LA MEJOR GLOBAL
    	//ShowEnjambre(Ejemplo);  //MOSTRAR EL ENJAMBRE Y LA MEJOR PARTICULA
    	srand(time(NULL));

    	while ((It<MaximoIteraciones)&&(50-Ejemplo->Enj[Ejemplo->idGbest].PFit)>0.000001)
    	{
        	ActualizarVelocidad(Ejemplo);
        	ActualizarPosicion(Ejemplo);
        	EvaluarEnjambre(Ejemplo);
        	ActualizarMejores(Ejemplo);
        	ShowEnjambre(Ejemplo);
        	printf("\n\nIteracion %i Best = %u",It,Ejemplo->idGbest);
        	It++;
    	}

    	ShowEnjambre(Ejemplo);
    	EliminarEnjambre(Ejemplo);
	printf("\n");

  	return 0;
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
