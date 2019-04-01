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
using namespace HM ;

const int c = 3, b = 2, npelados=5;
mutex
   mtx ;                 // mutex de escritura en pantalla

template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

class BarberiaSU : public HoareMonitor
{
 private:
   int numero_pelados, turno;
   CondVar         // colas condicion:
   clientes, barbero[b], silla;

 public:                    // constructor y métodos públicos
   BarberiaSU() ;           // constructor
   void  siguienteCliente( int i );                // extraer un valor (sentencia L) (consumidor)
   void cortarPelo( int i );
   void finCliente( );
} ;
// -----------------------------------------------------------------------------

BarberiaSU::BarberiaSU(  )
{
   numero_pelados=0, turno = 0;
   clientes = newCondVar();
   for ( int i = 0; i < b; ++i)
      barbero[i] = newCondVar();
   silla = newCondVar();
}
// -----------------------------------------------------------------------------
// función llamada por el consumidor para extraer un dato

void BarberiaSU::siguienteCliente( int i )
{
   if (clientes.empty() || turno != i )
      barbero[i].wait();
   clientes.signal();
}
// -----------------------------------------------------------------------------

void cortarPelo( int i )
{
   chrono::milliseconds duracion_pelar( aleatorio<20,200>());
   mtx.lock();
   cout << "Barbero " << i << " empieza a pelar" << endl;
   mtx.unlock();
   this_thread::sleep_for( duracion_pelar );
   mtx.lock();
   cout << "Fin del  barbero " << i << " (" << duracion_pelar.count() << " milisegundos)" << endl;
   mtx.unlock();
}
// *****************************************************************************
// funciones de hebras

void BarberiaSU::finCliente()
{
   ++numero_pelados;
   turno = (numero_pelados / npelados) % b;
   silla.signal();
}

void BarberiaSU::cortarPelo(int i)
{
   mtx.lock();
   cout << "Entra cliente: " << i << endl;
   mtx.unlock();

   if (barbero[turno].empty())
      clientes.wait();
   else
      barbero[turno].signal();
   silla.wait();

   mtx.lock();
   cout << "                      Sale cliente: " << i << endl;
   mtx.unlock();
}

void esperarFueraBarberia (int i)
{
   chrono::milliseconds duracion( aleatorio<20,200>());
   this_thread::sleep_for( duracion );
//   mtx.lock();
//   cout << "El cliente " << i << " ha estado fuera " << duracion.count() << " milisegundos" << endl;
//   mtx.unlock();
}  

void funcion_barbero( MRef<BarberiaSU> monitor, int i )
{
   while( true )
   {
      monitor->siguienteCliente(i);
      cortarPelo(i);
      monitor->finCliente();
   }
}
// -----------------------------------------------------------------------------

void funcion_cliente( MRef<BarberiaSU> monitor, int i )
{
   while( true )
   {
      monitor->cortarPelo(i);
      esperarFueraBarberia(i);
   }
}
// -----------------------------------------------------------------------------

int main()
{
   cout << "-------------------------------------------------------------------------------" << endl
        << "BARBERIA (BARBEROS TURNANDOSE) " << endl
        << "-------------------------------------------------------------------------------" << endl
        << flush ;

   auto monitor = Create <BarberiaSU>();

   thread barberos[b];
   thread clientes[c];

   for ( int i = 0; i < b ; ++i )
      barberos[i] = thread( funcion_barbero, monitor, i );
   for ( int i = 0; i < c ; ++i )
      clientes[i] = thread( funcion_cliente, monitor, i );

   for ( int i = 0; i < b ; ++i )
      barberos[i].join();
   for ( int i = 0; i < c ; ++i )
      clientes[i].join();
}
