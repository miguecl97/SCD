//Compilado con 
// g++ -std=c++11 -I. -pthread barbero.cpp HoareMonitor.cpp -o barbero.cpp 

#include <iostream>
#include <cassert>
#include <iomanip>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "HoareMonitor.hpp"

using namespace std ;
using namespace HM;


//**********************************************************************
// variables compartidas

mutex mtx ; // objeto mutex compartido
const int num_barberos = 2,
          num_clientes = 7;


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

//-------------------------------------------------------------------------
// Función que simula la acción de pelarse, como un retardo aleatoria de la hebra


void cortarPeloACliente()
{
   chrono::milliseconds duracion_pelar( aleatorio<20,200>() );
   this_thread::sleep_for( duracion_pelar );
   mtx.lock();
   cout << "El cliente ha tardado en cortarse el pelo " << duracion_pelar.count() << " milisegundos" <<endl;
   mtx.unlock();
}

void esperarFueraBarberia(int num_cliente)
{
   chrono::milliseconds duracion_pelar( aleatorio<50,500>() );
   this_thread::sleep_for( duracion_pelar );
   mtx.lock();
   cout << "Soy el cliente  " << num_cliente<< " y me he dado un paseo que ha durado " << duracion_pelar.count() << " milisegundos" <<endl;
   mtx.unlock();
}


class Barbero : public HoareMonitor
{
 private:
 CondVar                    // colas condicion:
   c_barbero[num_barberos],                //  cola donde espera el barbero
   c_cliente,                 //  cola donde espera el cliente
   c_silla[num_barberos];                  // cola de la silla
 public:                    // constructor y métodos públicos
   Barbero() ;           // constructor
   void cortarPelo(int num_cliente);
   void siguienteCliente(int num_barbero);
   void finCliente(int num_barbero);
   bool sillaLibre();
   int siguienteSilla();
} ;

Barbero::Barbero(  )
{
   
   for(int i=0; i<num_barberos; i++)
      c_barbero[i] = newCondVar();
   c_cliente = newCondVar();
   for(int i=0; i<num_barberos; i++)
      c_silla[i] = newCondVar();
}


bool Barbero::sillaLibre(){
   for(int i=0; i<num_barberos; i++)
      if(c_silla[i].empty())
         return true;
   return false;
}

int Barbero::siguienteSilla(){
   int siguiente;
   bool suficiente = false;
   for(int i=0; i<num_barberos && !suficiente; i++)
      if(c_silla[i].empty()){
         siguiente = i;
         suficiente = true;
   }
   return siguiente;
}

void Barbero::cortarPelo(int num_cliente){
   cout << "He entrado en la peluquería, soy el cliente número " << num_cliente << endl;
   if(c_cliente.get_nwt()!=0 || !sillaLibre()){
      cout << "Me siento y espero para poder pelarme" << endl;
      c_cliente.wait();
   }
   cout << "Soy el cliente número " << num_cliente << " y ya voy a cortarme el pelo" << endl;
   c_barbero[siguienteSilla()].signal();
   c_silla[siguienteSilla()].wait();
}

void Barbero::siguienteCliente(int num_barbero){
   if (c_silla[num_barbero].empty())
      c_barbero[num_barbero].wait();
}

void Barbero::finCliente(int num_barbero){
   c_silla[num_barbero].signal();
   c_cliente.signal();
}



//----------------------------------------------------------------------
// función que ejecuta la hebra del cliente
void funcion_hebra_barbero(MRef<Barbero>  monitor  , int num_barbero)
{
   while(true){
      monitor->siguienteCliente(num_barbero);
      cortarPeloACliente();
      monitor->finCliente(num_barbero);
   }
}



//----------------------------------------------------------------------
// función que ejecuta la hebra del cliente
void  funcion_hebra_cliente(MRef<Barbero>  monitor, int num_cliente)
{
   while( true )
   {
      monitor->cortarPelo(num_cliente);
      esperarFueraBarberia(num_cliente);
   }
}

//----------------------------------------------------------------------

int main()
{
   // declarar hebras y ponerlas en marcha
   // ......
   auto monitor = Create<Barbero>();
   thread barbero[num_barberos],
          cliente[num_clientes];

   for(int i=0; i<num_barberos; i++)
      barbero[i] = thread (funcion_hebra_barbero, monitor, i);

   for(int i=0; i<num_clientes; i++)
      cliente[i] = thread (funcion_hebra_cliente, monitor, i);

   for(int i=0; i<num_barberos; i++)
      barbero[i].join();

   for(int i=0; i<num_clientes; i++)
      cliente[i].join();
}
