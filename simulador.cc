/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <ns3/core-module.h>
#include <ns3/node.h>
#include <ns3/point-to-point-net-device.h>
#include <ns3/point-to-point-channel.h>
#include <ns3/drop-tail-queue.h>
#include <ns3/command-line.h>
#include <ns3/gnuplot.h>
#include <sstream>
#include <ns3/average.h>
#include "BitAlternante.h"

#define TAMPKT 994
#define NUMCURVAS 5
#define NUMPUNTOS 10

//numero de iteraciones para obtener el IC al 90%.
#define ITERACIONES_IC 10
//Parametro de t_student para IC del 90% con 10 grados de libertad.
#define TSTUDENT10 1.8331

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Practica03");

//Definicion de funciones
//Funcion que obtiene los datos para una curva simulando 
//NUMPUNTOS veces (NUMPUNTOS es el numero de puntos que tendra la curva).
//Recibe como parametros:
      //datosGrafica: puntero al objeto Gnuplot2dDataset donde se guardaran 
      //              los puntos de la curva.
      //tRetransmisionDesde: Valor inicial del temporizador de retransmisión.
      //tRetransmisionHasta: Valor final del temporizador de retransmisión.
      //tamPktB: tamaño de los paquetes enviados en bytes.
      //retPropagacion: retardo de propagacion asociado al canal.
      //velTxMedia: regimen binario con el que se transmitiran los datos.
void obtenerCurva (Gnuplot2dDataset *datosGrafica, Time tRetransmisionDesde, 
            Time tRetransmisionHasta, int tamPktB, 
            double retPropagacion, uint64_t velTx);


int
main (int argc, char *argv[])
{
  Time::SetResolution (Time::MS);

  //Varibales para los argumentos pasados porlinea de comandos:
  DataRate velocidadTxDesde = DataRate("0.5Mbps");
  DataRate velocidadTxHasta = DataRate("3Mbps");
  Time retardoPropDesde = Time("1ms");
  Time retardoPropHasta = Time("10ms");
  Time tRetransmisionDesde = Time("1ms");
  Time tRetransmisionHasta = Time("20ms");

  CommandLine cmd;
  
  //Preparamos la captura de valores por línea de comandos.
  cmd.AddValue("velocidadTxDesde","Velocidad de transmisión inicial.",velocidadTxDesde);
  cmd.AddValue("velocidadTxHasta","Velocidad de transmisión final.",velocidadTxHasta);
  cmd.AddValue("retardoPropDesde","Retardo de propagación inicial.",retardoPropDesde);
  cmd.AddValue("retardoPropHasta","Retardo de propagación final",retardoPropHasta);
  cmd.AddValue("tRetransmisionDesde","Valor inicial del temporizador de retransmisión.",tRetransmisionDesde);
  cmd.AddValue("tRetransmisionHasta","Valor final del temporizador de retransmisión",tRetransmisionHasta);
  cmd.Parse(argc,argv);

  //Calculamos los incrementos en las variables
  ////incremento en el retardo de propagacion para la grafica 1.
  double incrementoRetProp = (retardoPropHasta.GetDouble() - retardoPropDesde.GetDouble())/(NUMCURVAS-1);
  ////incremento en la velocidad de transmision para la grafica 2.
  double incrementoVelTx = (velocidadTxHasta.GetBitRate() - velocidadTxDesde.GetBitRate())/(NUMCURVAS-1);

  

  //Calculamos las variables medias necesarias
  ////velocidad de transmision media para la grafica 1
  uint64_t velTxMedia = (velocidadTxHasta.GetBitRate() + velocidadTxDesde.GetBitRate())/2;
  ////retardo de propagacion medio para la grafica 2
  double retPropagacionMedio = (retardoPropHasta.GetDouble() + retardoPropDesde.GetDouble())/2;

  //Preparamos la gráfica 1
  Gnuplot plot1;
  plot1.SetTitle("Gráfica 1 Práctica 03");

  //Iteramos 5 veces
  //5 valores para el retardo de propagacion. 
  for (double retPropagacion = retardoPropDesde.GetDouble(); 
        retPropagacion <= retardoPropHasta.GetDouble();
        retPropagacion += incrementoRetProp) 
  {

    //Para imprimir el rotulo de las curvas indicando el retardo
    std::stringstream sstm;
    sstm << "Retardo propagación: " << retPropagacion <<"ms";
    std::string result = sstm.str();

    Gnuplot2dDataset datos1 (result);
    datos1.SetStyle (Gnuplot2dDataset::LINES_POINTS);

    datos1.SetErrorBars (Gnuplot2dDataset::Y);

    //Obtenemos los datos de una curva por medio de 10 simulaciones
    obtenerCurva (&datos1, tRetransmisionDesde, tRetransmisionHasta, TAMPKT, retPropagacion, velTxMedia);

    //Añadimos los datos al objeto gráfica
    plot1.AddDataset(datos1);
  }
  
  //Abrimos el fichero de salida para la grafica 1
  std::ofstream fich("practica03-1.plt");
  //Añadimos los datos de la gráfica al fichero
  plot1.GenerateOutput(fich);
  //Añadimos esta linea al fichero para que la gráfica no se cierre nada más ejecutarse
  fich << "pause -1" << std::endl;
  //Cerramos el fichero
  fich.close();


  //Preparamos la gráfica 2
  Gnuplot plot2;
  plot2.SetTitle("Gráfica 2 Práctica 03");

  //Iteramos 5 veces para obtener 5 curvas que representaran
  //5 valores para la velocidad de transmision.
  for (uint64_t velocidadTx = velocidadTxDesde.GetBitRate(); 
        velocidadTx <= velocidadTxHasta.GetBitRate(); 
        velocidadTx += incrementoVelTx) 
  {

    //Para imprimir el rotulo de las curvas indicando el retardo
    std::stringstream sstm;
    sstm << "Velocidad de transmisión: " << velocidadTx/(1.0e6) <<"Mbps";
    std::string result = sstm.str();

    Gnuplot2dDataset datos2 (result);
    datos2.SetStyle (Gnuplot2dDataset::LINES_POINTS);

    datos2.SetErrorBars (Gnuplot2dDataset::Y);

    //Obtenemos los datos de una curva por medio de 10 simulaciones
    obtenerCurva (&datos2, tRetransmisionDesde, tRetransmisionHasta, TAMPKT, retPropagacionMedio, velocidadTx);

    //Añadimos los datos al objeto gráfica
    plot2.AddDataset(datos2);
  }
  
  //Abrimos el fichero de salida para la grafica 1
  std::ofstream fich2("practica03-2.plt");
  //Añadimos los datos de la gráfica al fichero
  plot2.GenerateOutput(fich2);
  //Añadimos esta linea al fichero para que la gráfica no se cierre nada más ejecutarse
  fich2 << "pause -1" << std::endl;
  //Cerramos el fichero
  fich2.close();
  return 0;
}


void obtenerCurva (Gnuplot2dDataset *datosGrafica, Time tRetransmisionDesde, 
                                  Time tRetransmisionHasta, int tamPktB, 
                                  double retPropagacion, uint64_t velTx) 
{
  //Variable donde guardaremos el error del intervalo de confianza.
  double z = 0.0;
  //Variable donde guardaremos 10 muestras de cada punto para obtener el intervalo de confianza.
  Average<uint32_t> ac_resultados;

  /////incremento en el temporizador de retransmision.
  double incrementoTempRet = (tRetransmisionHasta.GetDouble() - tRetransmisionDesde.GetDouble())/(NUMPUNTOS-1);
  
  //Iteramos 10 veces con distintos valores 
  //para el temporizador de retransmision
  for (double tempRetransmision = tRetransmisionDesde.GetDouble(); 
        tempRetransmision <= tRetransmisionHasta.GetDouble();
        tempRetransmision += incrementoTempRet) 
  {

    // Componentes del escenario:
    // Dos nodos
    Ptr<Node> nodoTx = CreateObject<Node> ();
    Ptr<Node> nodoRx = CreateObject<Node> ();
    // Dos dispositivos de red
    Ptr<PointToPointNetDevice> dispTx = CreateObject<PointToPointNetDevice> ();
    Ptr<PointToPointNetDevice> dispRx = CreateObject<PointToPointNetDevice> ();
    // Un canal punto a punto
    Ptr<PointToPointChannel> canal = CreateObject<PointToPointChannel> ();;
    // Una aplicación transmisora
    BitAlternanteTx transmisor(dispRx, Time(tempRetransmision), tamPktB);
    // Y una receptora
    BitAlternanteRx receptor(dispTx);

    // Montamos el escenario:
    // Añadimos una cola a cada dispositivo
    dispTx->SetQueue (CreateObject<DropTailQueue> ());
    dispRx->SetQueue (CreateObject<DropTailQueue> ());
    // Añadimos cada dispositivo a su nodo
    nodoTx->AddDevice (dispTx);
    nodoRx->AddDevice (dispRx);
    // Añadimos cada aplicación a su nodo
    nodoTx->AddApplication(&transmisor);
    nodoRx->AddApplication(&receptor);
    // Asociamos los dos dispositivos al canal
    dispTx->Attach (canal);
    dispRx->Attach (canal);
  
    // Modificamos los parámetos configurables
    canal->SetAttribute ("Delay", TimeValue(Time(retPropagacion)));
    dispTx->SetAttribute ("DataRate", 
            DataRateValue(DataRate(velTx)));

    // Activamos el transmisor
    transmisor.SetStartTime (Seconds (1.0));
    transmisor.SetStopTime (Seconds (10.0));

    //Hacemos ITERACIONES_IC iteraciones en el mismo punto para obetener
    //los datos para calcular el intervalo de confiaza al 90%.
    for (int iteracion = 0; iteracion <= ITERACIONES_IC; iteracion++)
    {
      NS_LOG_UNCOND ("Voy a simular con: " << tempRetransmision);
      Simulator::Run ();
      Simulator::Destroy ();

      //Guardamos el dato en la variable Average.
      ac_resultados.Update(transmisor.TotalDatos());

      NS_LOG_UNCOND ("Total paquetes: " << transmisor.TotalDatos());
    }

    //Calculamos el intervalo de confianza al 90%.
    z = TSTUDENT10*sqrt(ac_resultados.Var()/(ITERACIONES_IC));
    NS_LOG_INFO ("z = " << z);

    //Añadimos datos a la gráfica
    datosGrafica->Add(tempRetransmision, ac_resultados.Mean(), z);

    //Reseteamos la variable Average para la siguiente iteracion
    ac_resultados.Reset();

  }
}