// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Práctica 3. Implementación de algoritmos distribuidos con MPI
//
// Archivo: prodcons2.cpp
// Implementación del problema del productor-consumidor con
// un proceso intermedio que gestiona un buffer finito y recibe peticiones
// en orden arbitrario
// (versión con un único productor y un único consumidor)
//
// Historial:
// Actualizado a C++11 en Septiembre de 2017
// -----------------------------------------------------------------------------

#include <iostream>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <mpi.h>

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

const int
	num_consumidores = 5,
	num_productores = 4,
	
   id_buffer             = 0 ,
   num_procesos_esperado = num_productores + num_consumidores + 1 ,
   num_items             = 100, // multiplo de num_cons y num_prod
   tam_vector            = 40,
	num_itemsprod = num_items/num_productores,
	num_itemscons = num_items/num_consumidores,
	etiq_prod = 1, //0-np
	etiq_cons = 2, // np+1 np+nc-1
	etiq_buff = 0,  //np
	etiq_modo1 = 3,
	etiq_modo2 = 4;

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
// ptoducir produce los numeros en secuencia (1,2,3,....)
// y lleva espera aleatorio
int producir(int id_proceso)
{
   static int contador = (id_proceso-1) * num_itemsprod ;
   sleep_for( milliseconds( aleatorio<10,100>()) );
   contador++ ;
   cout << "Productor " << id_proceso << " ha producido valor " << contador << endl << flush;
   return contador ;
}
// ---------------------------------------------------------------------

void funcion_productor(int id_proceso)
{
   for ( unsigned int i= 0 ; i < num_itemsprod ; i++ )
   {
      int valor_prod = producir(id_proceso);
      cout << "Productor "<< id_proceso << " va a enviar valor " << valor_prod << endl << flush;
		if(id_proceso % 2 == 0)
     	   MPI_Ssend( &valor_prod, 1, MPI_INT, id_buffer, etiq_modo2, MPI_COMM_WORLD );
		else
			MPI_Ssend( &valor_prod, 1, MPI_INT, id_buffer, etiq_modo1, MPI_COMM_WORLD );
		
	
   }
}
// ---------------------------------------------------------------------

void consumir( int valor_cons, int id_proceso )
{
   // espera bloqueada
   sleep_for( milliseconds( aleatorio<110,200>()) );
   cout << "Consumidor "<< id_proceso << " ha consumido valor " << valor_cons << endl << flush ;
}
// ---------------------------------------------------------------------

void funcion_consumidor(int id_proceso)
{
   int         peticion,
               valor_rec = 1 ;
   MPI_Status  estado ;

   for( unsigned int i=0 ; i < num_itemscons; i++ )
   {
      MPI_Ssend( &peticion,  1, MPI_INT, id_buffer, etiq_cons, MPI_COMM_WORLD);
      MPI_Recv ( &valor_rec, 1, MPI_INT, id_buffer, etiq_buff, MPI_COMM_WORLD,&estado );
      cout << "Consumidor " << id_proceso<< " ha recibido valor " << valor_rec << endl << flush ;
      consumir( valor_rec, id_proceso);
   }
}
// ---------------------------------------------------------------------

void funcion_buffer()
{
   int        buffer[tam_vector],      // buffer con celdas ocupadas y vacías
              valor = 0,                   // valor recibido o enviado
              primera_libre       = 0, // índice de primera celda libre
              primera_ocupada     = 0, // índice de primera celda ocupada
              num_celdas_ocupadas = 0, // número de celdas ocupadas
				  cont = 1,
              tag_aceptable ;    // identificador de emisor aceptable
   MPI_Status estado ;                 // metadatos del mensaje recibido

   for( unsigned int i=0 ; i < num_items*2   ; i++ )
   {
         
		if ( num_celdas_ocupadas == tam_vector ) // si buffer lleno
         tag_aceptable = etiq_cons ;     
      else                                          // si no vacío ni lleno
			tag_aceptable = MPI_ANY_TAG ; 


		MPI_Probe( MPI_ANY_SOURCE, tag_aceptable, MPI_COMM_WORLD, &estado);		

		//	if( tag_aceptable == etiq_cons){
				if(estado.MPI_TAG == etiq_cons){			// consumir
					MPI_Recv( &valor, 1, MPI_INT, MPI_ANY_SOURCE, etiq_cons, MPI_COMM_WORLD, &estado );
					valor = buffer[primera_ocupada] ;
          	   primera_ocupada = (primera_ocupada+1) % tam_vector ;
          	   num_celdas_ocupadas-- ;
          	   cout << "Buffer va a enviar valor " << valor << endl ;
          	   MPI_Ssend( &valor, 1, MPI_INT, estado.MPI_SOURCE, etiq_buff, MPI_COMM_WORLD);

			//	}
         }else{
				if(cont % 2 !=0){   // modo1
					
  					 buffer[primera_libre] = valor ;
           		 primera_libre = (primera_libre+1) % tam_vector ;
           		 num_celdas_ocupadas++ ;
					 MPI_Recv( &valor, 1, MPI_INT, MPI_ANY_SOURCE, etiq_modo1, MPI_COMM_WORLD, &estado );					
					 cont++; 
           		 cout << "Buffer[modo1] ha recibido valor " << valor <<" desde prod "<< estado.MPI_SOURCE << endl ;
				}else{   			//modo2
					 buffer[primera_libre] = valor ;
           		 primera_libre = (primera_libre+1) % tam_vector ;
           		 num_celdas_ocupadas++ ;
					 MPI_Recv( &valor, 1, MPI_INT, MPI_ANY_SOURCE, etiq_modo2, MPI_COMM_WORLD, &estado );					 
					 cont++;
           		 cout << "Buffer[modo2] ha recibido valor " << valor << " desde prod "<< estado.MPI_SOURCE << endl ;

				}
			
			}
	
		}
}

// ---------------------------------------------------------------------

int main( int argc, char *argv[] )
{
   int id_propio, num_procesos_actual;

   // inicializar MPI, leer identif. de proceso y número de procesos
   MPI_Init( &argc, &argv );
   MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
   MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );

   if ( num_procesos_esperado == num_procesos_actual )
   {
      // ejecutar la operación apropiada a 'id_propio'
      if ( id_propio == id_buffer )
         funcion_buffer();
      else if ( id_propio <= num_productores )
         funcion_productor(id_propio);
      else
         funcion_consumidor(id_propio);
   }
   else
   {
      if ( id_propio == 0 ) // solo el primero escribe error, indep. del rol
      { cout << "el número de procesos esperados es:    " << num_procesos_esperado << endl
             << "el número de procesos en ejecución es: " << num_procesos_actual << endl
             << "(programa abortado)" << endl ;
      }
   }

   // al terminar el proceso, finalizar MPI
   MPI_Finalize( );
   return 0;
}
