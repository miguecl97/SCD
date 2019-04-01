// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Práctica 3. Implementación de algoritmos distribuidos con MPI
//
// Archivo: filosofos-plantilla.cpp
// Implementación del problema de los filósofos (sin camarero).
// Plantilla para completar.
//
// Historial:
// Actualizado a C++11 en Septiembre de 2017
// -----------------------------------------------------------------------------


#include <mpi.h>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <iostream>

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

const int
   num_filosofos = 5 ,
   num_procesos  = 2*num_filosofos+1,
	id_cam = 2*num_filosofos,
	etiq_coger = 0,
	etiq_dejar = 1,
	etiq_sentarse = 3,
	etiq_levantarse = 4,
   es_par = 5,
   es_impar = 6;


//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------

template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

// ---------------------------------------------------------------------

void funcion_filosofos( int id )
{
  int id_ten_izq = (id+1)              % (num_procesos-1), //id. tenedor izq.
      id_ten_der = (id+num_procesos-2) % (num_procesos-1); //id. tenedor der.
  int etiqueta;

  while ( true )
  {
	//SENTARSE
    // ... solicitar sentarse (completar)
	if( (id/2) % 2 == 0){
	 etiqueta = es_par;
	}
	else{
	 etiqueta = es_impar;
	}

	 cout <<"Filósofo " << id/2 << " solicita sentarse " <<endl;
    MPI_Ssend (NULL, 0, MPI_INT, id_cam, etiqueta, MPI_COMM_WORLD);
	 MPI_Ssend(NULL, 0, MPI_INT, id_cam, etiq_sentarse, MPI_COMM_WORLD);
	 cout <<"Filósofo. "<< id/2 << " ha sido sentado por camarero. " <<endl ;

    cout <<"Filósofo " << id/2 << " solicita ten. izq." <<id_ten_izq <<endl;
    // ... solicitar tenedor derecho (completar)
	 MPI_Ssend(NULL, 0, MPI_INT, id_ten_izq, etiq_coger, MPI_COMM_WORLD);

    cout <<"Filósofo " << id/2 <<" solicita ten. der." <<id_ten_der <<endl;
    // ... solicitar tenedor izquierdo (completar)
	 MPI_Ssend(NULL, 0, MPI_INT, id_ten_der, etiq_coger, MPI_COMM_WORLD);
	
	//COMER
    cout <<"Filósofo " << id/2 <<" comienza a comer" <<endl ;
    sleep_for( milliseconds( aleatorio<10,100>() ) );

    cout <<"Filósofo " << id/2 <<" suelta ten. izq. " <<id_ten_izq <<endl;
    // ... soltar el tenedor izquierdo (completar)
	 MPI_Ssend(NULL, 0, MPI_INT, id_ten_izq, etiq_dejar, MPI_COMM_WORLD);

    cout<< "Filósofo " << id/2 <<" suelta ten. der. " <<id_ten_der <<endl;
    // ... soltar el tenedor derecho (completar)
	 MPI_Ssend(NULL, 0, MPI_INT, id_ten_der, etiq_dejar, MPI_COMM_WORLD);

	//LEVANTARSE
    // ... solicitar levantarse (completar)
    MPI_Ssend (NULL, 0, MPI_INT, id_cam, etiqueta, MPI_COMM_WORLD);
	 MPI_Ssend(NULL, 0, MPI_INT, id_cam, etiq_levantarse, MPI_COMM_WORLD);
	 cout <<"Filósofo. "<< id/2 << " ha sido levantado por camarero. " <<endl;

	//PENSAR
    cout << "Filosofo " << id/2 << " comienza a pensar" << endl;
    sleep_for( milliseconds( aleatorio<10,100>() ) );
 }
}
// ---------------------------------------------------------------------

void funcion_tenedores( int id )
{
  int valor, id_filosofo ;  // valor recibido, identificador del filósofo
  MPI_Status estado ;       // metadatos de las dos recepciones

  while ( true )
  {
     // ...... recibir petición de cualquier filósofo (completar)
 		MPI_Recv(&valor, 1, MPI_INT, MPI_ANY_SOURCE, etiq_coger, MPI_COMM_WORLD, &estado);
     // ...... guardar en 'id_filosofo' el id. del emisor (completar)
		id_filosofo = estado.MPI_SOURCE;
     cout <<"Ten. " <<id <<" ha sido cogido por filo. " <<id_filosofo <<endl;

     // ...... recibir liberación de filósofo 'id_filosofo' (completar)
		MPI_Recv(&id_filosofo, 1, MPI_INT, id_filosofo, etiq_dejar, MPI_COMM_WORLD, &estado);
     cout <<"Ten. "<< id<< " ha sido liberado por filo. " <<id_filosofo <<endl ;
  }
}
// ---------------------------------------------------------------------

void funcion_camarero( int id )
{
  int valor,filo, etiq, s=0 ;  // valor recibido, identificador del filósofo
  MPI_Status estado ;       // metadatos de las dos recepciones
  int contador = 0;

  while ( true )
  {
	if( contador % 2 == 0)
		MPI_Recv (NULL, 0, MPI_INT, MPI_ANY_SOURCE, es_par, MPI_COMM_WORLD, &estado);
	else
		MPI_Recv (NULL, 0, MPI_INT, MPI_ANY_SOURCE, es_impar, MPI_COMM_WORLD, &estado);

	filo = estado.MPI_SOURCE;

	if (s < 4)
		MPI_Recv(NULL, 0, MPI_INT, filo, MPI_ANY_TAG, MPI_COMM_WORLD, &estado);
	else
		MPI_Recv(NULL, 0, MPI_INT, filo, etiq_levantarse, MPI_COMM_WORLD, &estado);

	etiq = estado.MPI_TAG;

	if (etiq == etiq_sentarse)
		s++;
	else
		s--;	

	contador++;
  }
}
// ---------------------------------------------------------------------

int main( int argc, char** argv )
{
   int id_propio, num_procesos_actual ;

   MPI_Init( &argc, &argv );
   MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
   MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );


   if ( num_procesos == num_procesos_actual )
   {
		if ( id_propio == id_cam)
			funcion_camarero( id_propio );
      // ejecutar la función correspondiente a 'id_propio'
      else if ( id_propio % 2 == 0 )          // si es par
         funcion_filosofos( id_propio ); //   es un filósofo
      else                               // si es impar
         funcion_tenedores( id_propio ); //   es un tenedor
   }
   else
   {
      if ( id_propio == 0 ) // solo el primero escribe error, indep. del rol
      { cout << "el número de procesos esperados es:    " << num_procesos << endl
             << "el número de procesos en ejecución es: " << num_procesos_actual << endl
             << "(programa abortado)" << endl ;
      }
   }

   MPI_Finalize( );
   return 0;
}

// ---------------------------------------------------------------------
