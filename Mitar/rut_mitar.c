#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include "mitar.h"

extern char *uso;
int copynFile(FILE *origen, FILE *destino, int nBytes);
int readHeader(FILE *tarFile, stHeaderEntry **header, int *nFiles);
int loadstr( FILE *file, char** buf );
int createTar(int nFiles, char *fileNames[], char tarName[]);
int extractTar(char tarName[]);




/** Copia el nBytes del fichero origen al fichero destino.
 *
 * origen: puntero al descriptor FILE del fichero origen
 * destino: puntero al descriptor FILE del fichero destino
 * nBytes: numero de bytes a copiar
 *
 * Devuelve el numero de bytes copiados o -1 en caso de error. 
 */
int copynFile(FILE *origen, FILE *destino, int nBytes){
  int copiados = 0;
  char byte;
  int lee = 1;
  while((lee != 0) && copiados < nBytes)
  {
    lee = fread(&byte,1,1,origen);
    if (lee != 0)
    {
      copiados += fwrite(&byte,1,1,destino);
    }
  }
    

  
  return copiados;
}

/** Carga en memoria la cabecera del tarball.
 *
 * tarFile: puntero al descriptor FILE del tarball
 * header: direcci�n de un puntero que se inicializar� con la direcci�n de un
 * buffer que contenga los descriptores de los ficheros cargados.
 * nFiles: direcci�n de un entero (puntero a entero) que se inicializar� con el
 * n�mero de ficheros contenido en el tarball (primeros bytes de la cabecera).
 *
 * Devuelve EXIT_SUCCESS en caso de exito o EXIT_FAILURE en caso de error
 * (macros definidas en stdlib.h).
 */
int readHeader(FILE *tarFile, stHeaderEntry **header, int *nFiles){
  int leido = 0;
  int leido2 = 0;
  printf("readHeader\n");
  leido = fread(nFiles,sizeof(int),1,tarFile);
  printf("readHeader: leido = %d", leido);
  printf("readHeader: nFile = %d",*nFiles);
  
  if (leido != 1)
  {
    return EXIT_FAILURE;
  }
  *header = (stHeaderEntry*) malloc(sizeof(stHeaderEntry) * (*nFiles));
  
  //pongo variables fuera por que no uso c11;
  
  int i;
  for ( i= 0; i< *nFiles; i++)
  {
    
    leido = loadstr(tarFile,&(*header)[i].name);
    leido2 = fread(&(*header)[i].size,sizeof(int),1,tarFile);
    if (leido != 0 || leido2 != 1)
    {
      return EXIT_FAILURE;
    }
  }
  
  
  return EXIT_SUCCESS;
  
}

/** Carga una cadena de caracteres de un fichero.
 *
 * file: direcci�n de la estructura FILE del fichero
 * buf: direcci�n del puntero en que se inicializar� con la direcci�n donde se
 * copiar� la cadena. Ser� una direcci�n del heap obtenida con malloc.
 *
 * Devuelve: 0 si tuvo exito, -1 en caso de error.
 */
int loadstr( FILE *file, char** buf ){
  char nombre[FILENAME_MAX];
  int leido = 0;
  char c;
  do {
    if (leido == FILENAME_MAX)
    {
      return -1;
    }
 
    leido += fread(&nombre[leido],sizeof(char),1,file);
    c = nombre[leido-1];
  }  while(c != '\0');
  *buf = (char*)malloc(leido*sizeof(char));
  strncpy(*buf,nombre,(leido-1));
  //si NO hay algun error
  if ((*buf)[leido] == '\0')
  {
    return 0;
  }
  else return -1;
}

/** crea un tarball a partir de unos ficheros de entrada.
 *
 * nfiles:    numero de ficheros de entrada
 * filenames: array con los nombres de los ficheros de entrada (rutas)
 * tarname:   nombre del tarball
 * 
 * devuelve exit_success en caso de exito y exit_failure en caso de error
 * (macros definidas en stdlib.h).
 *
 * ayuda: calcular primero el espacio necesario para la cabecera del tarball,
 * saltar dicho espacio desde el comienzo del archivo e ir copiando uno a uno
 * los ficheros, rellenando a la vez una representaci�n de la cabecera del
 * tarball en memoria. al final copiar la cabecera al comienzo del tarball.
 * recordar que  que no vale un simple sizeof de stheaderentry para saber el
 * tamano necesario en disco para el descriptor de un fichero puesto que sizeof
 * no calcula el tamano de la cadena apuntada por name, sino el tamano del
 * propio puntero. para calcular el tamano de una cadena c puede usarse la
 * funci�n strlen.
 */
int createTar(int nFiles, char *fileNames[], char tarName[]) {
  FILE *fichero;
  fichero = fopen(tarName,"w+");
  int escrito;
  //variables para guardar los strings de cabeceras y los tama�os de los archivos
  int *tamanos = (int*) malloc(sizeof(int) * (nFiles));
  int *nombres =(int*) malloc(sizeof(int) * (nFiles));
  
  //inicializado a int por que el minimo sera el entero que dice el numero de archivos a comprimir
  int tamanocabecera = sizeof(unsigned int);
  //calculo tamano cabecera para ver cuanto tengo que reservar
  //pongo variables fuera por que no uso c11;
  int i;
  for ( i = 0; i < nFiles; i++)
  {
    nombres[i] =strlen(fileNames[i])+ 1;
    tamanocabecera += nombres[i] + sizeof(unsigned int);
  }
  
  //muevo el puntero pasada la cabecera
  fseek(fichero,tamanocabecera,SEEK_SET);
  
  //por cada archivo muevo sus datos
  for(i = 0;i < nFiles; i++)
  {
    FILE *archivo;
    escrito = 0;
    //arbo el archivo "i" , copio sus datos y lo cierro
    archivo = fopen(fileNames[i],"r");
    escrito = copynFile(archivo, fichero, UINT_MAX);
    fclose(archivo);
    
    //si no da error guardo el tama�o de lo copiado para rellenar luego la cabecera
    if (escrito<0)
    {
      goto error;
    }
    else 
    {
      tamanos[i] = escrito;
    }
    
  }
  //muevo el puntero a la cabecera
  fseek(fichero,0,SEEK_SET);
  
  //copio la cabecera
  //copio primero el numero de archivos
  escrito = fwrite(&nFiles,sizeof(int),1,fichero);
  
  if (escrito != 1)
  {
    goto error;
  }
  
  //ahora si no hay error copio el resto de la cabecera
  for(i = 0;i < nFiles; i++)
  {

    escrito = fwrite(fileNames[i],sizeof(char),nombres[i],fichero);
    if (escrito != nombres[i])
    {
      goto error;
    }
    
    escrito = fwrite(&tamanos[i],sizeof(int),1,fichero);
    
    if (escrito != 1)
    {
      goto error;
    }
    //si no da error guardo el tama�o de lo copiado para rellenar luego la cabecera

    
  }
  free(tamanos);
  free(nombres);
  fclose(fichero);
  return EXIT_SUCCESS;
  
  
  
  
  error:
  free(tamanos);
  free(nombres);
  fclose(fichero);
  return EXIT_FAILURE;
  
}

/** Extrae todos los ficheros de un tarball.
 *
 * tarName: cadena C con el nombre del tarball
 *
 * Devuelve EXIT_SUCCES en caso de exito y EXIT_FAILURE en caso de error (macros
 * definidas en stdlib.h).
 *
 * AYUDA: debemos cargar primero la cabecera, con lo que nos situaremos al
 * comienzo del primer fichero almacenado. Entonces recorriendo la cabecera
 * sabremos el nombre que debemos ir dando a los ficheros y la cantidad de bytes
 * que debemos leer del fichero para cargar su contenido.
 */
int extractTar(char tarName[]) {
 printf("extractTar\n");
  
 FILE *fichero;
 int nFiles = 0;
 
 stHeaderEntry *header;
 
 fichero = fopen(tarName,"r+");

 readHeader(fichero, &header, &nFiles);

 //pongo variables fuera por que no uso c11;
 int i;
 int j;
 for (i = 0; i< nFiles; i++)
 {
   FILE *archivo = fopen(header[i].name,"w+");
   int escrito = copynFile(fichero, archivo, header[i].size);
   fclose(archivo);
   
   if(escrito != header[i].size)
   {	
     //pongo variables fuera por que no uso c11;
      for (j = 0; j< nFiles; j++)
      {
	free(header[j].name);
      }
      free(header);
     return EXIT_FAILURE;
   }
   
 }
 //pongo variables fuera por que no uso c11;
 for (i = 0; i< nFiles; i++)
 {
   free(header[i].name);
 }
 
 free(header);
 
 return EXIT_SUCCESS;
}
  
