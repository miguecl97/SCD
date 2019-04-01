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
using namespace std::this_thread;
using namespace std::chrono;

const int
   num_filosofos = 5,
   num_procesos = 2 * num_filosofos + 1, // un tenedor por cada filósofo + el camarero[0,1,...,10]
   id_camarero = num_procesos - 1, // es el proceso numero 10
   etiq_sentarsepar = 0,
   etiq_levantarse = 1,
	etiq_tenedor = 2,
	etiq_sentarseimpar = 4;

//----------------------------------------------------------------------

template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max );
  return distribucion_uniforme( generador );
}


//----------------------------------------------------------------------

void funcion_filosofos( int id )
{
  int id_ten_izq = (id + 1) % (num_procesos-1),
      id_ten_der = (id + num_procesos - 2) % (num_procesos-1),
      valor,
		etiq_sentarse;
  chrono::milliseconds espera( aleatorio<20,200>() );

	if( (id/2) % 2 == 0){
		etiq_sentarse = etiq_sentarsepar;
	}else{
		etiq_sentarse = etiq_sentarseimpar;
	}

  while ( true )
  {
    // 1. Sentarse

    cout << "Filósofo " << id/2 << " solicita permiso para sentarse a la mesa ." << endl;
    MPI_Ssend( &valor, 1, MPI_INT, id_camarero, etiq_sentarse, MPI_COMM_WORLD );

    // 2. Pedir tenedores

    cout << "Filósofo " << id/2 << " solicita tenedor izquierdo " << id_ten_izq << " ." << endl;
    MPI_Ssend( &valor, 1, MPI_INT, id_ten_izq, etiq_tenedor, MPI_COMM_WORLD );
	 cout << "Filósofo " << id/2 << " solicita tenedor derecho " << id_ten_der << " ." << endl;
    MPI_Ssend( &valor, 1, MPI_INT, id_ten_der, etiq_tenedor, MPI_COMM_WORLD );

    // 3. Comer

    cout << "\nFlósofo " << id/2 << " empieza a comer . \n" << endl;
	 this_thread::sleep_for( espera );
	
    // 4. Soltar tenedores

    cout << "Filósofo " << id/2 << " suelta tenedor izquierdo ." << id_ten_izq << "" << endl;
    MPI_Ssend( &valor, 1, MPI_INT, id_ten_izq, etiq_tenedor, MPI_COMM_WORLD );
	 cout << "Filósofo " << id/2 <<" suelta tenedor derecho ." << id_ten_der << "" << endl;
    MPI_Ssend( &valor, 1, MPI_INT, id_ten_der, etiq_tenedor, MPI_COMM_WORLD );

    // 5. Levantarse

    cout << "Filósofo " << id/2 << " solicita permiso para levantarse ." << endl;
    MPI_Ssend( &valor, 1, MPI_INT, id_camarero, etiq_levantarse, MPI_COMM_WORLD );

    // 6. Pensar
	 cout << "Filósofo " << id/2 << " comienza a pensar . "<< endl;
    this_thread::sleep_for( espera );
  }
}

// ---------------------------------------------------------------------

void funcion_camarero()
{
  int filosofos_sentados = 0,  // número de filósofos sentados en la mesa
      id_filosofo,    
		contador,      
		etiquete,   
      etiqueta_aceptable;  

	MPI_Status estado;
   while( true )
   {
     if ( filosofos_sentados < num_filosofos - 1 ){
			if( contador % 2 ==0){
				etiquete = etiq_sentarsepar;
			}else{
				etiquete = etiq_sentarseimpar;
			}

			MPI_Recv( &id_filosofo, 1, MPI_INT, MPI_ANY_SOURCE, etiquete , MPI_COMM_WORLD, &estado );
			cout << " CLiente , iteracion " << contador;
			id_filosofo = estado.MPI_SOURCE;
			etiqueta_aceptable = MPI_ANY_TAG;     
     }else                                      
        etiqueta_aceptable = etiq_levantarse;   
  		   


	  cout << "Filósofo " << id_filosofo << " se sienta de la mesa ." << endl;
	 // MPI_Recv( &id_filosofo, 1, MPI_INT, MPI_ANY_SOURCE, etiquete , MPI_COMM_WORLD, &estado );
     MPI_Recv( &id_filosofo, 1, MPI_INT, MPI_ANY_SOURCE, etiqueta_aceptable, MPI_COMM_WORLD, &estado );
		
    // id_filosofo = estado.MPI_SOURCE;
		
     switch( estado.MPI_TAG ) // leer etiqueta del mensaje en metadatos
     {
        case etiq_levantarse:
           cout << "Filósofo " << id_filosofo << " se levanta de la mesa ." << endl;
           filosofos_sentados--;
			  contador++;
           break;

        case etiq_sentarsepar:
           cout << "Filósofo " << id_filosofo << " se sienta a la mesa ." << endl;
           filosofos_sentados++;
			  contador++;
           break;
			
			case etiq_sentarseimpar:
           cout << "Filósofo " << id_filosofo << " se sienta a la mesa ." << endl;
           filosofos_sentados++;
			  contador++;
           break;
     }	
		cout << "\n Iteracion num " << contador << endl;

   }
}

// ---------------------------------------------------------------------

void funcion_tenedores( int id )
{
  int        valor,
             id_filosofo ;
  MPI_Status estado ;

  while ( true )
  {
  
     MPI_Recv( &valor, 1, MPI_INT, MPI_ANY_SOURCE, etiq_tenedor, MPI_COMM_WORLD, &estado );
     id_filosofo = estado.MPI_SOURCE;
     cout << "\tTenedor " << id <<" cogido por filósofo " << id_filosofo << endl;

     MPI_Recv( &valor, 1, MPI_INT, id_filosofo, etiq_tenedor, MPI_COMM_WORLD, &estado );
     cout << "\tTenedor " << id << " soltado por filósofo " << id_filosofo << endl;
  }
}

//----------------------------------------------------------------------

int main( int argc, char** argv )
{
   int id_propio, num_procesos_actual;

   MPI_Init( &argc, &argv );
   MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
   MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );

   if ( num_procesos == num_procesos_actual )
   {
      if ( id_propio == id_camarero )
         funcion_camarero();
      else if ( id_propio % 2 == 0 )     // si es par
         funcion_filosofos( id_propio ); // es un filósofo
      else                               // si es impar
         funcion_tenedores( id_propio ); // es un tenedor
   }
   else
   {
      if ( id_propio == 0 )
      {
  		  cout << "El número de procesos esperados es:    " << num_procesos << endl
             << "El número de procesos en ejecución es: " << num_procesos_actual << endl
				 << "(Programa abortado)." << endl;
      }
   }

   MPI_Finalize();
   return 0;
}
