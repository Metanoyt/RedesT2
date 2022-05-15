# Redes Tarea 2

## <font size="6">Cómo correr la tarea</font>

### <font size="5">**En Windows**</font>

1. Instalar Docker y Docker Compose, para esto se puede apoyar en la siguiente página: <https://docs.docker.com/desktop/windows/install/>
    - Puede descargar el ejecutable que incluse Docker y Docker Compose en: <https://desktop.docker.com/win/main/amd64/Docker%20Desktop%20Installer.exe>
2. Entrar a la carpeta del proyecto, utilizando la consola de windows y el comando 'cd' desplazarse en los directorios del sistema hasta encontrar la carpeta donde se encuentra el proyecto, también se puede utilizar la dirección completa del archivo.
3. Una vez se tiene la consola ubicada dentro de la dirección del proyecto utilizar el comando:  

    - ```bash
      docker-compose up --build
      ```

    - Algunas veces en Windows es necesario añadir manualmente la carpeta de \Docker\resources\bin a la
      variable de entorno PATH para que reconozca el comando ***docker-compose***.

### <font size="5">**En linux**</font>

#### <font size="4">**Configurar repositorio:**</font>

1. Actualizar la lista de paquetes con:

    - ```bash
      sudo apt-get update
      ```

2. Permitir apt usar un repositorio por medio de HTTPS

    - ```bash
      sudo apt-get install \
      ca-certificates \
      curl \
      gnupg \
      lsb-release
      ```

3. Añadir la llave oficial GPG de Docker
   - ```curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo --dearmor -o /usr/share/keyrings/docker-archive-keyring.gpg```
4. Configurar la rama stable del Repositorio:

    - ```bash
      echo \
      "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/docker-archive-keyring.gpg] https://download.docker.com/linux/ubuntu \
      $(lsb_release -cs) stable" | sudo tee /etc/apt/sources.list.d/docker.list > /dev/null
      ```

#### <font size="4">**Instalar Docker Engine:**</font>

1. Instalamos la última versión de Docker Engine, containerd y Docker Compose:

    - ```bash
      sudo apt-get update && \
      sudo apt-get install docker-ce docker-ce-cli containerd.io docker-compose-plugin
      ```

#### <font size="4">**Instalar Docker Compose:**</font>

1. Usamos este comando para descargar la última versión estable de Docker Compose:

    - ```bash
      DOCKER_CONFIG=${DOCKER_CONFIG:-$HOME/.docker} && \
      mkdir -p $DOCKER_CONFIG/cli-plugins && \
      curl -SL https://github.com/docker/compose/releases/download/v2.4.1/docker-compose-linux-x86_64 -o && \
      $DOCKER_CONFIG/cli-plugins/docker-compose
      ```

2. Damos permisos para ejecutar el instalador:

    - ```bash
      chmod +x $DOCKER_CONFIG/cli-plugins/docker-compose
      ```

3. Probamos la instalación con:

    - ```bash
      docker compose version
      ```

#### <font size="4">**Iniciar Docker Compose:**</font>

  1. Entramos en la carpeta con el archivo *docker-compose.yaml* y ejecutamos el comando:

      - ```bash
        docker-compose up --build
        ```

## Descripcion de la solucion

### Servidor ubuntu

Para la automatización de ubuntu se crea un contenedor con una imagen de ubuntu, operando bajo una red de tipo bride bajo el puerto 9666, después de inicializada en el contenedor se instalan las bibliotecas de gcc y make para inicializar el servidor.  

### Operaciones

#### Broadcast

Una vez se tiene un arreglo de 4 bytes que representa el valor numérico de la ip de entrada, es posible calcular la ip de broadcast para la red simbólica de la misma ip, esta ip de broadcast consiste en el valor más alto posible que puede alcanzar bajo las restricciones dadas por su máscara de bits, para calcularla se puede aplicar un operador bitwise "or" con la negación "~" de la máscara de bits.

#### Network Number

Para calcular el número de red para una combinación de ip y máscara se aplica un operador bitwise de tipo AND "&"  entre los dos.

#### Host range

Para calcular un rango de IPs posibles a quienes "hostear", se debe encontrar el valor que no tengan conflicto con la máscara de red en cada byte de la ip, para esto se puede utilizar el conjunto de operaciones de "Ip & ~ mask" si el valor resultante de esta operación es mayor a 0 significa que es posible usar al menos una combinación de bits para asignarla a un huésped, para encontrar el valor máximo disponible para cada byte sin tener conflicto con el ip de broadcast se puede aplicar la operación
"~mask[i] & 254" sobre el byte i de la máscara de bits, al hacerlo encontramos el mayor número que no tiene conflicto con la máscara de red y el aplicar el operador "and" con el valor 254 nos asegura que el valor no será mayor a 254 osease 255.  

## Referencias

### Codigo utilizado

Se utilizo este ejemplo de servidor echo como esqueleto para el servidor de la tarea: <https://mohsensy.github.io/programming/2019/09/25/echo-server-and-client-using-sockets-in-c.html>
