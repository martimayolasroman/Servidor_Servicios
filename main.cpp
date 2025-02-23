
#include <SFML/Network.hpp>
#include <iostream>
#include <string>
#include "Servidor_Servicios.h"
#include <thread>
#include <memory>








int main() {


	
	try {
		Servidor_Servicios servidor;
		servidor.iniciarServidor();
		servidor.procesarMensajes(); // Bucle infinito
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
	
	
	return 0;
}