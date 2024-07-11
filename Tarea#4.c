#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

// Definición de la estructura para almacenar los parámetros del estudiante
typedef struct {
    char nombre[50];
    int edad;
    int nota;
} Parametros;

// Prototipos de funciones
void grabarDato(void);
void leerDisco(void);
void borrarDato(void);

// Función principal
int main() {
    int opcion;

    // Bucle para mantener el menú activo hasta que el usuario decida salir
    do {
        // Mostrar las opciones del menú
        printf("\t*********Menu:**********\n");
        printf("1- Grabar Un Dato\n");
        printf("2- Leer El Disco\n");
        printf("3- Borrar Un Dato\n");
        printf("4- Salir\n");
        printf("Seleccionar: ");
        scanf("%d", &opcion); // Leer la opción del usuario

        // Ejecutar la función adecuada según la opción del usuario
        if (opcion == 1) {
            grabarDato();
            system("cls");
        } else if (opcion == 2) {
            leerDisco();
            system("pause");
            system("cls");
        } else if (opcion == 3) {
            borrarDato();
            system("pause");
            system("cls");
        } else if (opcion == 4) {
            system("cls");
            printf("\n\nSALIENDO DEL PROGRAMA.\n\n");
        } else {
            printf("Opción no válida.\n");
            system("pause");
            system("cls");
        }
    } while (opcion != 4);

    return 0;
}

// Función para grabar datos del estudiante en el archivo
void grabarDato(void) {
    FILE *archivo = fopen("alumno.dat", "ab"); // Abrir el archivo para agregar datos binarios

    // Verificar si el archivo se abrió correctamente
    if (!archivo) {
        perror("Error al abrir el archivo");
        return;
    }

    // Asignar memoria para una estructura de estudiante
    Parametros *alumno = (Parametros *)malloc(sizeof(Parametros));
    if (!alumno) {
        perror("Error al asignar memoria");
        fclose(archivo);
        return;
    }

    char bucle = 's'; // Variable para controlar el bucle de entrada

    // Bucle para registrar múltiples estudiantes si se desea
    while (bucle == 's') {
        // Ingresar datos del estudiante
        printf("Ingrese el nombre del alumno: ");
        scanf("%s", alumno->nombre);
        fflush(stdin);
        printf("Ingrese la edad del alumno: ");
        scanf("%d", &alumno->edad);
        fflush(stdin);
        printf("Ingrese la nota del alumno: ");
        scanf("%d", &alumno->nota);
        fflush(stdin);

        // Escribir los datos del estudiante en el archivo
        fwrite(alumno, sizeof(Parametros), 1, archivo);

        // Preguntar si el usuario desea ingresar otro registro
        printf("Desea ingresar otro registro? (s/n): ");
        scanf(" %c", &bucle);
    }

    // Liberar la memoria asignada y cerrar el archivo
    free(alumno);
    fclose(archivo);
}

// Función para leer datos del estudiante desde el archivo
void leerDisco(void) {
    FILE *archivo = fopen("alumno.dat", "rb"); // Abrir el archivo para leer datos binarios

    // Verificar si el archivo se abrió correctamente
    if (!archivo) {
        perror("Error al abrir el archivo");
        return;
    }

    // Asignar memoria para una estructura de estudiante
    Parametros *alumno = (Parametros *)malloc(sizeof(Parametros));
    if (!alumno) {
        perror("Error al asignar memoria");
        fclose(archivo);
        return;
    }

    // Leer y mostrar cada registro de estudiante del archivo
    while (fread(alumno, sizeof(Parametros), 1, archivo)) {
        printf("Nombre: %s, Edad: %d, Nota: %d\n", alumno->nombre, alumno->edad, alumno->nota);
    }

    // Liberar la memoria asignada y cerrar el archivo
    free(alumno);
    fclose(archivo);
}

// Función para borrar un dato específico del archivo
void borrarDato(void) {
    FILE *archivo = fopen("alumno.dat", "rb"); // Abrir el archivo para leer datos binarios
    if (!archivo) {
        perror("Error al abrir el archivo");
        return;
    }

    // Asignar memoria para una estructura de estudiante
    Parametros *alumno = (Parametros *)malloc(sizeof(Parametros));
    if (!alumno) {
        perror("Error al asignar memoria");
        fclose(archivo);
        return;
    }

    // Crear un array dinámico para almacenar todos los registros
    Parametros *alumnos = NULL;
    int count = 0;

    // Leer todos los registros del archivo
    while (fread(alumno, sizeof(Parametros), 1, archivo)) {
        count++;
        alumnos = (Parametros *)realloc(alumnos, count * sizeof(Parametros));
        if (!alumnos) {
            perror("Error al reasignar memoria");
            free(alumno);
            fclose(archivo);
            return;
        }
        alumnos[count - 1] = *alumno;
    }

    fclose(archivo);
    free(alumno);

    // Mostrar los registros y pedir al usuario que elija cuál eliminar
    for (int i = 0; i < count; i++) {
        printf("%d. Nombre: %s, Edad: %d, Nota: %d\n", i + 1, alumnos[i].nombre, alumnos[i].edad, alumnos[i].nota);
    }

    int index;
    printf("Ingrese el número del registro que desea borrar: ");
    scanf("%d", &index);
    index--; // Ajustar a índice basado en 0

    // Verificar si el índice es válido
    if (index < 0 || index >= count) {
        printf("Índice no válido.\n");
        free(alumnos);
        return;
    }

    // Abrir el archivo para escribir (borrar todo y reescribir)
    archivo = fopen("alumno.dat", "wb");
    if (!archivo) {
        perror("Error al abrir el archivo");
        free(alumnos);
        return;
    }

    // Escribir todos los registros excepto el que se desea eliminar
    for (int i = 0; i < count; i++) {
        if (i != index) {
            fwrite(&alumnos[i], sizeof(Parametros), 1, archivo);
        }
    }

    fclose(archivo);
    free(alumnos);

    printf("El registro ha sido borrado.\n");
}

