/* Copyright 2015, Pablo Ridolfi
 *
 * This file is part of TD2-Template.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

/** @brief This is a simple C example file.
 **
 **/

/** \addtogroup TD2 Técnicas Digitales II
 ** @{ */

/** @addtogroup App Aplicación de usuario
 * 	@{
 */

/*
 * Initials     Name
 * ---------------------------
 * PR           Pablo Ridolfi
 */

/*
 * modification history (new versions first)
 * -----------------------------------------------------------
 * 20150421 v0.0.1   PR first version
 */

/*==================[inclusions]=============================================*/

#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

#include <cr_section_macros.h>

#include "main.h"
/*==================[macros and definitions]=================================*/
#define MPU6050_DEVICE_ADDRESS   0x68
#define MPU6050_RA_ACCEL_XOUT_H  0x3B
#define MPU6050_RA_PWR_MGMT_1    0x6B
#define MPU6050_RA_PWR_MGMT_2    0x6C
#define MPU6050_PWR1_SLEEP_BIT   6

#define DEBUGOUT(...) printf(__VA_ARGS__)

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/** @brief hardware initialization function
 *	@return none
 */

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

static void initHardware(void)
{
#if defined (__USE_LPCOPEN)
#if !defined(NO_BOARD_LIB)
    // Read clock settings and update SystemCoreClock variable
    SystemCoreClockUpdate();
    // Set up and initialize all required blocks and
    // functions related to the board hardware
    Board_Init();
    // Set the LED to the state of "Off"
    Board_LED_Set(0, false);
#endif
#endif
	Chip_IOCON_PinMux(LPC_IOCON, 0, 19, IOCON_MODE_INACT, IOCON_FUNC2);
	Chip_IOCON_PinMux(LPC_IOCON, 0, 20, IOCON_MODE_INACT, IOCON_FUNC2);
	Chip_IOCON_EnableOD(LPC_IOCON, 0, 19);
	Chip_IOCON_EnableOD(LPC_IOCON, 0, 20);

    /* pines del stick */
	Chip_IOCON_PinMux(LPC_IOCON, 0, 0, IOCON_MODE_INACT, IOCON_FUNC3);
	Chip_IOCON_PinMux(LPC_IOCON, 0, 1, IOCON_MODE_INACT, IOCON_FUNC3);
	Chip_IOCON_EnableOD(LPC_IOCON, 0, 0);
	Chip_IOCON_EnableOD(LPC_IOCON, 0, 1);

    Chip_I2C_SetClockRate(I2C1, 100000);
	Chip_I2C_SetMasterEventHandler(I2C1, Chip_I2C_EventHandlerPolling);
}

/*==================[external functions definition]==========================*/

//TODO: Leer 10 veces el registro de Acelerometro en X, esto nos dara 5 datos
//	  	compuestos de 2 bytes cada uno (parte high y low)


int main(void)
{
	/*==================[Inicializacion]==========================*/
	uint8_t wbuf[2] = {0,0};
	uint8_t rbuf[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	//Se lo crea con 6 posiciones porque los registros que se procesan de los sensores son :
	/*
	ACCEL_XOUT_H
	ACCEL_XOUT_L
	ACCEL_YOUT_H
	ACCEL_YOUT_L
	ACCEL_ZOUT_H
	ACCEL_ZOUT_L
	TEMP_OUT_H
	TEMP_OUT_L
	GYRO_XOUT_H
	GYRO_XOUT_L
	GYRO_YOUT_H
	GYRO_YOUT_L
	GYRO_ZOUT_H
	GYRO_ZOUT_L
	 */

	uint16_t samples[7] = {0,0,0,0,0,0,0}; //cada posicion es de 16 bits, necesario para guardar
											//la parte low y high de las muestras de accel

	uint32_t promX = 0;
	uint32_t promY = 0;
	uint32_t promZ = 0;

	I2C_XFER_T xfer;

	initHardware();

	/*==================[Configuracion del I2C_XFER_T]==========================*/

	// Metodo de Escritura
	//Define el registro que se va a leer en la primera posicion de wbuf
	//wbuf[0] = MPU6050_RA_ACCEL_XOUT_H; //Parte high de la lectura en x del acelerometro
							//Como la lectura se realiza de forma secuencial, en la posicion 0
							//del rbuf ira este dato y en la posicion 1 ira la correspondiente a
							//la posicion siguiente, osea ACCEL_XOUT_L (0x3C)

	MPU6050_wakeup(&xfer);

	//Lectura de PWR_MGMMT_1 2 (para verificar si se lo saco del sleep y de standby a los ejes)
	wbuf[0] = MPU6050_RA_PWR_MGMT_1;
	I2C_XFER_config(&xfer, rbuf, 2, MPU6050_I2C_SLAVE_ADDRESS, 0, wbuf, 1);

	//Configuracion de la 1era direccion desde la que se leeran los valores de los registros de los sensores
	wbuf[0]=MPU6050_RA_ACCEL_XOUT_H;
	wbuf[1]=0;

	int32_t i, k;

	DEBUGOUT("Prueba acelerometro..\n");

	while(1)
	{

		promX = promY = promZ = 0;

		for ( k = 0 ; k < 100 ; k ++ )
		{
			I2C_XFER_config(&xfer, rbuf, 14, MPU6050_I2C_SLAVE_ADDRESS, 0, wbuf, 1);

			Fill_Samples(&samples, &rbuf);

			promX += samples[4];
			promY += samples[5];
			promZ += samples[6];

			for ( i=0 ; i<25000 ; i++ );
		}

		promX /= 100; promY /= 100; promZ /= 100;

		if ( promX < 5000 )
			DEBUGOUT("Aviso en eje X con valor : %d \n", promX );

		if ( promY < 5000 )
			DEBUGOUT("Aviso en eje Y con valor : %d \n", promY );

		if ( promZ < 5000 )
			DEBUGOUT("Aviso en eje Z con valor : %d \n", promZ );

		//for ( k=0 ; k<1000 ; k++ );

		//DEBUGOUT("Aviso en posicion %d con valor : %d \n", 4, samples[4] );
		//DEBUGOUT("%d          %d          %d          %d          %d          %d          %d\n",samples[0],samples[1],samples[2],samples[3],samples[4],samples[5],samples[6] );
/*
		for ( i=0 ; i<1 ; i++ )
		{
			if ( samples[i] < 3000 )
			{
				if ( i != 4 )
					DEBUGOUT("Aviso en señal %d con valor : %d \n", i, samples[i] );
			}
		}
*/
	}

	return 0;
}

/*
 * Configura la estructura XFER para realizar la comunicacion i2c.
 * * xfer	  : Puntero a la estructura del tipo I2C_XFER_T necesaria para utilizar la funcion Chip_I2C_MasterTransfer.
 * 				Chip_I2C_MasterTransfer : Funcion que resuelve la interaccion i2c en funcion de lo especificado en la estructura I2C_XFER_T
 *	 rbuf 	  : Puntero al buffer de lectura donde se volcaran los bytes leidos
 *	 rxSz 	  : Cantidad de bytes que se leeran y volcaran en rbuf
 *	 slaveAddr: Direccion estatica del slave con el que se desea comunicar
 *	 status   : Estado de la comunicacion, (estado inicial 0)
 *	 wbuf	  : Buffer de escritura donde se colocara tanto el registro que se desea escribir como el dato que desea ser escrito
 *	 			Ej de uso: wbuf[] = {reg_inicial, dato} solo escribe el byte dato en reg_inicial
 *	 					   wbuf[] = {reg_inicial, dato1, dato2} escribe el byte dato1 en reg_incial y dato2 en reg_inicial+1 (el registro siguiente)
 *	 txSz	  : La cantidad de bytes que se desean enviar, osea empezando a leer wbuf desde 0 inclusive, cuantos bytes manda de ese buffer
 *	 			Ej : wbuf[] = {reg_inicial, dato1, dato2}, entonces txSz deberia ser = 3
 *	 				 wbuf[] = {reg_inicial}, (caso tipico de solo lectura de ese registro), entonces txSz deberia ser = 1
 */

void I2C_XFER_config (I2C_XFER_T * xfer,uint8_t *rbuf, int rxSz, uint8_t slaveAddr, I2C_STATUS_T status, uint8_t * wbuf, int txSz)
{
	xfer->rxBuff = rbuf; //Buffer de lectura
	xfer->rxSz = rxSz;	//cantidad de bytes que se desean leer, arbitrariamente seteamos 10
	xfer->slaveAddr = slaveAddr; //Adress estatica del dispositivo i2c a leer (MPU6050)
	xfer->status = status;
	xfer->txBuff = wbuf; //Buffer de escritura
	xfer->txSz = txSz; //cantidad de bytes que se desean escribir, solo escribimos el registro desde
					//el que comenzamos a leer
	Chip_I2C_MasterTransfer(I2C1, xfer);
}

/*
 * Se encarga de inicializar los registros de PWR_MGMT necesarios para habilitar los
 *  sensores (acelerometro y giroscopo) en cada eje, para sus lecturas.
 *  La configuracion en la que quedan seteados es la por defecto:
 *  accelerometer (±2g) , gyroscope (±250°/sec).
 *
 *  * xfer : Puntero a la estructura del tipo I2C_XFER_T necesaria para la utilizacion de Chip_I2C_MasterTransfer.
 *  		 Chip_I2C_MasterTransfer : Funcion que resuelve la interaccion i2c en funcion de lo especificado en la estructura I2C_XFER_T
 */
void MPU6050_wakeup(I2C_XFER_T * xfer)
{
		//Setea PWR_MGMT_1 y 2 en 0, el byte de cada uno


		uint8_t wbuf[3] = {MPU6050_RA_PWR_MGMT_1, 0, 0};
		/*xfer->slaveAddr = MPU6050_DEVICE_ADDRESS;
		xfer->txBuff = wbuf;
		xfer->txSz = 3;
		xfer->rxSz = 0;*/

		I2C_XFER_config(xfer, xfer->rxBuff, 0, MPU6050_I2C_SLAVE_ADDRESS, 0, wbuf, 3);
}

/* Llena el vector de muestras samples con la data de los registros de ACCEL, GYRO y TEMP del MPU
 * Al estar la informacion en 16 bits y ser levantada por registros de 8, en rbuf esta la parte low
 * 	y high de cada componente, por lo que se debe recomponer desplazando la parte high y concatenando la low]
 * 	para obtener el valor que se midio. Proceso que se realiza en esta funcion para pasar a samples.
 *
 * 	 rbuf : Tiene la data leida por el MPU con la data dividida en high y low
 * 	 samples : Tendra la data agrupada que representa al valor medido por los sensores en cada eje
 */
void Fill_Samples(uint16_t * samples, uint8_t * rbuf)
{
	//De momento leer rbuf es lo mismo que leer xfer.rxBuff porque apuntan a la misma direccion
	//Desplazamos la parte alta a los 8 ultimos bits y le hacemos un OR para tener en los 8 primeros la parte baja


	samples[0]=(rbuf[0] << 8) | rbuf[1];
	samples[1]=(rbuf[2] << 8) | rbuf[3];
	samples[2]=(rbuf[4] << 8) | rbuf[5];
	samples[3]=(rbuf[6] << 8) | rbuf[7];
	samples[4]=(rbuf[8] << 8) | rbuf[9];
	samples[5]=(rbuf[10] << 8) | rbuf[11];
	samples[6]=(rbuf[12] << 8) | rbuf[13];
}

/** @} doxygen end group definition */
/** @} doxygen end group definition */

/*==================[end of file]============================================*/


