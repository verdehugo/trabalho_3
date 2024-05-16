/**
 * @file main.c
 * @brief Implementation of a Vending Machine (Assignment_3)
 *
 * SETR 23/24
 * Hugo Verde, 72705, verde.hugo@ua.pt
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "../Machine_Vending/src/sc_types.h"
#include "../Machine_Vending/src-gen/VM.h"
#include "../Machine_Vending/src-gen/VM_required.h"    // Prototypes of external functions called by the SM
#include <zephyr/kernel.h>          /* for k_msleep() */
#include <zephyr/device.h>          /* for device_is_ready() and device structure */
#include <zephyr/devicetree.h>      /* for DT_NODELABEL() */
#include <zephyr/drivers/gpio.h>    /* for GPIO api*/

VM VMStateMachine;  // The statemachine structure variable

/* Get led0 ... led3 and button0..button3 node IDs. Refer to the DST file*/
#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)
#define LED3_NODE DT_ALIAS(led3)
#define BUT0_NODE DT_ALIAS(sw0)
#define BUT1_NODE DT_ALIAS(sw1)
#define BUT2_NODE DT_ALIAS(sw2)
#define BUT3_NODE DT_ALIAS(sw3)

/* Get the device pointer, pin number, and configuration flags for led1..led4 and button 1... button4. A build error on this line means your board is unsupported. */

/* Define an array to hold LED configurations */
static const struct gpio_dt_spec led[]= {
    GPIO_DT_SPEC_GET(LED0_NODE, gpios),
    GPIO_DT_SPEC_GET(LED1_NODE, gpios),
    GPIO_DT_SPEC_GET(LED2_NODE, gpios),
    GPIO_DT_SPEC_GET(LED3_NODE, gpios)
};
/* Define an array to hold button configurations */
static const struct gpio_dt_spec but[]={
    GPIO_DT_SPEC_GET(BUT0_NODE, gpios),
    GPIO_DT_SPEC_GET(BUT1_NODE, gpios),
    GPIO_DT_SPEC_GET(BUT2_NODE, gpios),
    GPIO_DT_SPEC_GET(BUT3_NODE, gpios)
};


/**
 * @brief Callback function for button press event
 *
 * @param dev The device structure
 * @param cb The gpio callback structure
 * @param pins The pin number
 */
void buttonPressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
    if (pins & BIT(but[0].pin)) {
        vM_but_raise_coin1(&VMStateMachine); // Raise coin1 event
    } else if (pins & BIT(but[1].pin)) {
        vM_but_raise_coin2(&VMStateMachine); // Raise coin2 event
    } else if (pins & BIT(but[2].pin)) {
        vM_but_raise_browse(&VMStateMachine); // Raise browse event
    } else if (pins & BIT(but[3].pin)) {
        vM_but_raise_enter(&VMStateMachine); // Raise enter event
    } else {
        // Do nothing
    }
}


/**
 * @brief Main function
 */
int main(void){
    int ret = 0;
    static struct gpio_callback buttonCbData; // Define variable for holding callback data

    // Initialize the state machine
    vM_init(&VMStateMachine);

    // Enter the state machine
    vM_enter(&VMStateMachine);

    /* Check if devices are ready */
    for (int i = 0; i < 4; i++) {
        if (!device_is_ready(led[i].port) || !device_is_ready(but[i].port)) {
            return 0;
        }
    }

    /* Configure the GPIO pins for input/output and set active logic */
    /* Note that the devicetree activates the internal pullup and sets the active low flag */
    /*   so an external resistor is not needed and pressing the button causes a logic level of 1*/
    for (int i = 0; i < 4; i++) {
        ret = gpio_pin_configure_dt(&led[i], GPIO_OUTPUT_INACTIVE);
    }

    if (ret < 0) {
        return 0;
    }

    for (int i = 0; i < 4; i++) {
        ret = gpio_pin_configure_dt(&but[i], GPIO_INPUT);
    }
    if (ret < 0) {
        return 0;
    }

    /* Set interrupt */
    /* Set interrupt for each button and assign the single callback function */
    for(int i = 0; i<4; i++){
        ret = gpio_pin_interrupt_configure_dt(&but[i],GPIO_INT_EDGE_TO_ACTIVE);
    }

    gpio_init_callback(&buttonCbData, buttonPressed, BIT(but[0].pin) | BIT(but[1].pin) |BIT(but[2].pin) | BIT(but[3].pin));

    for (int i = 0; i < 4; i++) {
        gpio_add_callback(but[i].port, &buttonCbData);
    }

    return 0;
}


/**
 * @brief Welcome message function
 *
 * @param handle The handle of the statemachine
 */
void vM_welcome_message(VM* handle){
    printk("\n");
    printk("Bem vindo ! \n Insira dinheiro ou veja os produtos disponíveis !\n");

    /* Activate LEDs to indicate that Vending Machine is ready */
    gpio_pin_set(led[0].port, led[0].pin, 1); 
    gpio_pin_set(led[1].port, led[1].pin, 1);
    gpio_pin_set(led[2].port, led[2].pin, 1);
    gpio_pin_set(led[3].port, led[3].pin, 1);
}

/**
 * @brief Function to select a product
 *
 * @param handle The handle of the statemachine
 * @param product The product to be selected
 * @param credit The current credit
 */
void vM_select_product(VM* handle, const sc_integer product, const sc_integer credit){
    // Activate LEDs to indicate which product is selected
    if(product == 0){
        gpio_pin_set(led[0].port, led[0].pin, 0); 
        gpio_pin_set(led[1].port, led[1].pin, 0);
        gpio_pin_set(led[2].port, led[2].pin, 0); 
        gpio_pin_set(led[3].port, led[3].pin, 0);

        printk("\n");
        printk("Return Credit selected\n");
        printk("Current credit is %d Euros\n",credit);
    }

    if(product == 1){
        gpio_pin_set(led[0].port, led[0].pin, 1); 
        gpio_pin_set(led[1].port, led[1].pin, 0);

        printk("\n");
        printk("Product price is 1 Euro\n");
        printk("Current credit is %d Euro\n", credit);
    }
    if(product == 2){
        gpio_pin_set(led[0].port, led[0].pin, 0); 
        gpio_pin_set(led[1].port, led[1].pin, 1);

        printk("\n");
        printk("Product price is 2 Euros\n");
        printk("Current credit is %d Euros\n", credit);
    }
    if(product == 3){
        gpio_pin_set(led[0].port, led[0].pin, 1); 
        gpio_pin_set(led[1].port, led[1].pin, 1);

        printk("\n");
        printk("Product price is 3 Euros\n");
        printk("Current credit is %d Euros\n", credit);
    }
}

/**
 * @brief Function to dispense a product
 *
 * @param handle The handle of the statemachine
 * @param product The product to be dispensed
 */
void vM_dispense_product(VM* handle, const sc_integer product){
    // Activate LED3 to indicate that product has been dispensed
    gpio_pin_set(led[0].port, led[0].pin, 0); 
    gpio_pin_set(led[1].port, led[1].pin, 0);
    gpio_pin_set(led[2].port, led[2].pin, 1);
    gpio_pin_set(led[3].port, led[3].pin, 0);

    printk("\n");
    printf("Product %d dispensed ! \n", product);
}

/**
 * @brief Function to return credit
 *
 * @param handle The handle of the statemachine
 * @param credit The credit to be returned
 */
void vM_return_credit(VM* handle, const sc_integer credit){
    // Activate LED4 to indicate that return credit is available
    gpio_pin_set(led[0].port, led[0].pin, 0); 
    gpio_pin_set(led[1].port, led[1].pin, 0);
    gpio_pin_set(led[2].port, led[2].pin, 0);
    gpio_pin_set(led[3].port, led[3].pin, 1);

	printk("\n");
    printf("Credit %d returned!\n",  credit);
}
