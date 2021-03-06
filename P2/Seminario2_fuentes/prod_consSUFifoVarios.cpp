// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Seminario 2. Introducción a los monitores en C++11.
//
// archivo: prodcons_1.cpp
// Ejemplo de un monitor en C++11 con semántica SC, para el problema
// del productor/consumidor, con un único productor y un único consumidor.
// Opcion LIFO (stack)
//
// Historial:
// Creado en Julio de 2017
// -----------------------------------------------------------------------------


#include <iostream>
#include <iomanip>
#include <cassert>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <random>
#include "HoareMonitor.hpp"

using namespace std ;
using namespace HM;
constexpr int
   num_items  = 40,    // número de items a producir/consumir
   num_consumidores = 5,
   num_productores = 2;
mutex
   mtx ;                 // mutex de escritura en pantalla
unsigned
   cont_prod[num_items], // contadores de verificación: producidos
   cont_cons[num_items]; // contadores de verificación: consumidos

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

//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

int producir_dato()
{
   static int contador = 0 ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));
   mtx.lock();
   cout << "producido: " << contador << endl << flush ;
   mtx.unlock();
   cont_prod[contador] ++ ;
   return contador++ ;
}
//----------------------------------------------------------------------

void consumir_dato( unsigned dato )
{
   if ( num_items <= dato )
   {
      cout << " dato === " << dato << ", num_items == " << num_items << endl ;
   }
   cont_cons[dato] ++ ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));
   mtx.lock();
   cout << "                  consumido: " << dato << endl ;
   mtx.unlock();
}
//----------------------------------------------------------------------

void ini_contadores()
{
   for( unsigned i = 0 ; i < num_items ; i++ )
   {  cont_prod[i] = 0 ;
      cont_cons[i] = 0 ;
   }
}

//----------------------------------------------------------------------

void test_contadores()
{
   bool ok = true ;
   cout << "comprobando contadores ...." << flush ;

   for( unsigned i = 0 ; i < num_items ; i++ )
   {
      if ( cont_prod[i] != 1 )
      {
         cout << "error: valor " << i << " producido " << cont_prod[i] << " veces." << endl ;
         ok = false ;
      }
      if ( cont_cons[i] != 1 )
      {
         cout << "error: valor " << i << " consumido " << cont_cons[i] << " veces" << endl ;
         ok = false ;
      }
   }
   if (ok)
      cout << endl << flush << "solución (aparentemente) correcta." << endl << flush ;
}

// *****************************************************************************
// clase para monitor buffer, version LIFO, semántica SC, un prod. y un cons.

class ProdCons1SU : public HoareMonitor
{
 private:
 static const int           // constantes:
   num_celdas_total = 10;   //  núm. de entradas del buffer
 int                        // variables permanentes
   buffer[num_celdas_total],//  buffer de tamaño fijo, con los datos
   primera_libre ,         //  indice de celda de la próxima inserción
   n,
   primera_ocupada;         //  indice de celda de la próxima inserción
 mutex
   cerrojo_monitor ;        // cerrojo del monitor
 CondVar         // colas condicion:
   ocupadas,                //  cola donde espera el consumidor (n>0)
   libres ;                 //  cola donde espera el productor  (n<num_celdas_total)

 public:                    // constructor y métodos públicos
   ProdCons1SU(  ) ;           // constructor
   int  leer();                // extraer un valor (sentencia L) (consumidor)
   void escribir( int valor ); // insertar un valor (sentencia E) (productor)
} ;
// -----------------------------------------------------------------------------

ProdCons1SU::ProdCons1SU(  )
{
   primera_libre = 0 ;
   n=0;
   primera_ocupada=0;
   ocupadas = newCondVar();
   libres = newCondVar();
}
// -----------------------------------------------------------------------------
// función llamada por el consumidor para extraer un dato

int ProdCons1SU::leer(  )
{
   // ganar la exclusión mutua del monitor con una guarda
   //unique_lock<mutex> guarda( cerrojo_monitor );

   // esperar bloqueado hasta que 0 < num_celdas_ocupadas
   if ( n == 0 )
      ocupadas.wait( );

   // hacer la operación de lectura, actualizando estado del monitor
   assert( 0 < n  );
   n-- ;
   const int valor = buffer[primera_ocupada] ;
   primera_ocupada = (primera_ocupada + 1 ) %  num_celdas_total;


   // señalar al productor que hay un hueco libre, por si está esperando
   libres.signal();

   // devolver valor
   return valor ;
}
// -----------------------------------------------------------------------------

void ProdCons1SU::escribir( int valor )
{
   // ganar la exclusión mutua del monitor con una guarda
   //unique_lock<mutex> guarda( cerrojo_monitor );

   // esperar bloqueado hasta que num_celdas_ocupadas < num_celdas_total
   if ( n == num_celdas_total )
      libres.wait( );

   assert( n < num_celdas_total );

   // hacer la operación de inserción, actualizando estado del monitor
   buffer[primera_libre] = valor ;
   n++;
   primera_libre =  (primera_libre+1) % num_celdas_total;

   // señalar al consumidor que ya hay una celda ocupada (por si esta esperando)
   ocupadas.signal();
}
// *****************************************************************************
// funciones de hebras

void funcion_hebra_productora( MRef<ProdCons1SU> monitor )
{
   for( unsigned i = 0 ; i < num_items/num_productores ; i++ )
   {
      int valor = producir_dato() ;
      monitor->escribir( valor );
   }
}
// -----------------------------------------------------------------------------

void funcion_hebra_consumidora( MRef<ProdCons1SU> monitor )
{
   for( unsigned i = 0 ; i < num_items/num_consumidores ; i++ )
   {
      int valor = monitor->leer();
      consumir_dato( valor ) ;
   }
}
// -----------------------------------------------------------------------------

int main()
{
   cout << "-------------------------------------------------------------------------------" << endl
        << "Problema de los productores-consumidores ("<< num_productores <<" prod /" << num_consumidores << " cons, Monitor SU, buffer FIFO). " << endl
        << "-------------------------------------------------------------------------------" << endl
        << flush ;
   auto monitor = Create<ProdCons1SU>() ;

   thread hebras_consumidoras[num_consumidores];
   thread hebras_productoras[num_productores];

   for(int i=0; i<num_productores;i++){
	hebras_productoras[i]= thread(funcion_hebra_productora, monitor);
   }

   for(int i=0; i<num_consumidores;i++){
	hebras_consumidoras[i]= thread(funcion_hebra_consumidora, monitor);
   }

   for(int i=0; i<num_consumidores;i++){
	hebras_consumidoras[i].join();
   }

   for(int i=0; i<num_productores;i++){
	hebras_productoras[i].join();
   }

   // comprobar que cada item se ha producido y consumido exactamente una vez
   test_contadores() ;
}
