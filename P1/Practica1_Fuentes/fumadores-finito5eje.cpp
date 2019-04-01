/**
 * Pedro Manuel Flores Crespo
 * Sistemas Concurrentes y distribuidos
 * Solución sl problema de los fumadores
 */


#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <vector>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "Semaphore.h"

using namespace std ;
using namespace SEM ;

//**********************************************************************
// variables compartidas
Semaphore mostrador_vacio(1);     //Semáforo para saber si el mostrador está vací
Semaphore todos_terminado[3] ={0,0,0};     //Avisar que todos han terminado de fumar
Semaphore ingredientes_disponibles[3]={0,0,0};   //Semáforo para cada uno de los ingredientes
const int num_producciones = 5;
int elementos_producidos;

/** Compilar
 * g++ -std=c++11 -I. -o ejecutable fuente1.cpp Semaphore.cpp -lpthread
 */

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

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero(  )
{
  int ingrediente;
  for(elementos_producidos=0; elementos_producidos<num_producciones; ++elementos_producidos){
    ingrediente = aleatorio<0,2>();     //Producimos el ingrediente
    sem_wait(mostrador_vacio);          //Vemos si el mostrador está vacío
    cout << "En el mostrador puesto ingrediente: " << ingrediente << endl;    //Ponemos el ingrediente
    sem_signal(ingredientes_disponibles[ingrediente]);    //Avisamos al cliente de dicho ingrediente
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
   static bool despertado = false;
   while( elementos_producidos < num_producciones || !despertado)
   {
        sem_wait(ingredientes_disponibles[num_fumador]);   //Esperamos a que está nuestro producto
        if(!despertado){
          cout << "Retirado ingrediente: " << num_fumador << endl;   //Lo quitamos
          sem_signal(mostrador_vacio);     //Le decimos al estanquero que el mostrador está vacío
          fumar(num_fumador);   //Fumamos
        }

        if(elementos_producidos == num_producciones && !despertado){
          despertado = true;
          for(int i=0; i<3; ++i){
            if(i != num_fumador){
              sem_signal(ingredientes_disponibles[i]);
            }
          }
        }
   }
}

//----------------------------------------------------------------------

int main()
{
  cout << "--------------------------------------------------------" << endl
  << "Problema de los fumadores." << endl
  << "--------------------------------------------------------" << endl
  << flush ;

  thread estanquero ( funcion_hebra_estanquero );   //Lanzamos la hebra estanquero

  thread fumadores[3];    //Lanzamos las hebras clientes
  for(int i=0; i<3; ++i){
    fumadores[i] = thread(funcion_hebra_fumador, i);
  }

  estanquero.join();   //Las unimos
  for(int i=0; i<3; ++i){
    fumadores[i].join();
  }
}
