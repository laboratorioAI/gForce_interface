En este directorio se encuentra el código en C++ para generación de la función MEX interfaz del brazalete GForce-Pro con Matlab.
Siga la **Guía de instalación.pdf** para generación de la función C++ MEX.
Al haber terminado el proceso de instalación, puede copiar unicamente la carpeta **export_mexMatlab** que contiene los archivos necesarios.

# Contenido
## gForce_mexVS
Proyecto en Visual Studio 2019 con el código C++ para generación de la función MEX.
NOTA: Esta carpeta no es necesaria para la ejecución en Matlab, por lo que puede eliminarla después de la compilación de la función C++ MEX (ya que Visual Studio genera muchos archivos (>200Mb) intermedios).


## export_mexMatlab
Carpeta en la que se exportará la función C++ MEX y que contiene las dependencias necesarias.

## app_mat_mex_v0
Carpeta con un proyecto de Visual Studio para compiliación de una función C++ MEX sin dependencias al SDK de GForce. Ideado para probar que la compilación esté correcta.

# Créditos
Laboratorio de Investigación en Inteligencia y Visión Artificial
ESCUELA POLITÉCNICA NACIONAL
Quito - Ecuador

autor: ztjona!
jonathan.a.zea@ieee.org

>  "I find that I don't understand things unless I try to program them."
-Donald E. Knuth

30 December 2020
Matlab 9.9.0.1538559 (R2020b) Update 3.
