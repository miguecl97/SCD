// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Practica 2. Introducción a los monitores en C++11.
//
// archivo: barbero_su.cpp
//
//
// -----------------------------------------------------------------------------


#include <iostream>
#include <iomanip>
#include <random>
#include <mutex>
#include "HoareMonitor.hpp"

using namespace std ;
using namespace HM ;

constexpr int clientes = 10;  //numero de clientes para la barberia
constexpr int n_sillas = 5; // numero de sillas en la sala de espera
mutex m1; //mutex para asegurar que no se solapen las salidas por pantalla

template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

void CortarPeloACliente(){
   chrono::milliseconds duracion_pelar( aleatorio<20,200>() );
	m1.lock();
   cout << "Cliente pelado en (" << duracion_pelar.count() << " milisegundos)" << endl;
	m1.unlock();
   this_thread::sleep_for( duracion_pelar );
}

void EsperarFueraBarberia( int cli ){
   chrono::milliseconds duracion_esperar( aleatorio<20,200>() );
	m1.lock();
   cout << "Cliente "<< cli << " espera fuera durante (" << duracion_esperar.count() << " milisegundos)\n" << endl;
	m1.unlock();
   this_thread::sleep_for( duracion_esperar );
}

// *****************************************************************************
// clase para monitor Barberia,  semántica SU

class Barberia : public HoareMonitor
{
   private:
   CondVar cond_clientes,   // cola de clientes	
	   	  cond_barbero,    //controla cuando el barbero duerme
      	  cond_silla;     // controla la silla donde el barbero pela
   int contador;
   public:
   Barberia( ) ; // constructor
   void cortarPelo( int cli );
   void siguienteCliente( );
   void finCliente( );
   int getContador();
} ;
// -----------------------------------------------------------------------------

Barberia::Barberia(  )
{
   cond_clientes = newCondVar();
   cond_barbero = newCondVar();
   cond_silla = newCondVar();
}
// -----------------------------------------------------------------------------

void Barberia::cortarPelo( int cli )
{

   if(cond_clientes.get_nwt() < n_sillas)
 	{
		m1.lock();
   	cout << "Cliente numero " << cli << " entra a la Barberia." << endl;
		m1.unlock();

      if(cond_clientes.get_nwt() != 0 || cond_silla.get_nwt() != 0)
     		cond_clientes.wait();
  	 	
		m1.lock();
      cout << "Cliente numero " << cli << " se sienta." << endl;
		m1.unlock();

  		cond_barbero.signal();
  		cond_silla.wait();
		
	}
	else
		m1.lock();
		cout << "Cliente numero " << cli << " no puede entrar porque la sala de espera esta llena, esta muy ENFADADO ." << endl;
		m1.unlock();
		
}

void Barberia::siguienteCliente( )
{
   if(cond_silla.get_nwt() == 0){
		m1.lock();
      cout << "Barbero se duerme...\n" << endl;
		cond_barbero.wait();
		m1.unlock();
      //cond_barbero.wait();  
   }

   
}

void Barberia::finCliente( )
{
	m1.lock();
   cout << "				Barbero termina y Cliente se levanta dejando la silla libre." << endl;
	m1.unlock();
   cond_silla.signal();
   cond_clientes.signal();

}

// -----------------------------------------------------------------------------

void  funcion_hebra_cliente( MRef<Barberia> monitor, int ncliente )
{
   while( true )
   {
      monitor->cortarPelo( ncliente );
      EsperarFueraBarberia( ncliente );
   }
}

void funcion_hebra_barbero( MRef<Barberia> monitor )
{
   while( true ){
      monitor->siguienteCliente();
      CortarPeloACliente();
      monitor->finCliente();
   }
}

// *****************************************************************************

int main()
{
   // crear monitor
   cout << "Solucion al problema de  la barberia con monitores SU, con " << clientes << " clientes. " << endl;
   auto monitor = Create<Barberia>( );

	
   thread hebra_clientes[clientes], hebra_barbero;
   for( unsigned i = 0 ; i < clientes ; i++ )
   {
      hebra_clientes[i] = thread( funcion_hebra_cliente, monitor, i );
   }

   hebra_barbero = thread( funcion_hebra_barbero, monitor );

   for( unsigned i = 0 ; i < clientes ; i++ )
   {
      hebra_clientes[i].join();
   }
   hebra_barbero.join();
}
