#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <future>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "Semaphore.h"

using namespace std ;
using namespace SEM ;

//**********************************************************************
//g++ -std=c++11 -I. -o fumadores fumadores.cpp Semaphore.cpp -lpthread

// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------

// variables compartidas

Semaphore mostr_vacio=1;
Semaphore ingr_disp[3]={0,0,0};
Semaphore ingrediente_terminado[3] = {0,0,0};
int ingredientes_totales = 5;
int contador_ingredientes;

template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

//-------------------------------------------------------------------------
// función producir, produce ingrediente aleatorio dado por el estanquero

int producir(){
  // calcular milisegundos aleatorios de duración de la acción de dispensar un ingrediente)
   chrono::milliseconds duracion_dispensar( aleatorio<20,200>() );
   int ingrediente( aleatorio<0,2>() );
  // espera bloqueada un tiempo igual a ''duracion_dispensar' milisegundos
   this_thread::sleep_for( duracion_dispensar );
   return ingrediente;
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero(  )
{
   int i;
   for ( contador_ingredientes = 0 ; contador_ingredientes < ingredientes_totales; contador_ingredientes++)
   {
      i = producir();
      sem_wait( mostr_vacio );
      cout << "Puesto ingr.: " << i << endl;
      sem_signal( ingr_disp[i] );
   }
}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador )
{

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

   // informa de que comienza a fumar

    cout << "Fumador " << num_fumador << "  :"
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar

    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;

}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void  funcion_hebra_fumador( int num_fumador )
{
   bool fin = false;
   while(contador_ingredientes < ingredientes_totales || !fin)
   {
   	sem_wait(ingr_disp[num_fumador]);
	if(!fin){
		cout << "Retirado ingr.: " << num_fumador << endl;
		sem_wait( ingr_disp[b] );
                cout << "Retirado ingr.: " << b << endl;
                sem_signal( mostr_vacio );
                fumar( num_fumador );
  	 } 
	if(contador_ingredientes == ingredientes_totales && !fin){
		fin = true;
		for(int=0; i < 3 ; ++i){
			if( i != num_fumador){
				sem_signal(ingredientes_disponibles[i]);
			}
		}
	}
}

//----------------------------------------------------------------------

int main()
{
    // declarar hebras y ponerlas en marcha
   cout << "--------------------------------------------------------" << endl
	<< "Problema de los Fumadores." << endl
	<< "--------------------------------------------------------" << endl
	<< flush ;

   thread hebra_estanquero;
   hebra_estanquero = thread( funcion_hebra_estanquero );
   thread hebra_fumador[3] ;
   
   for( int i = 0 ; i < 3 ; i++ )
	hebra_fumador[i] = thread( funcion_hebra_fumador, i ) ;
	
   hebra_estanquero.join();
   for( int i = 0 ; i < 3 ; i++ )
	hebra_fumador[i].join();

}
