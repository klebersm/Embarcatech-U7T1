#include "U7T1.h"

// Alarmes para controle de debounce dos botões
static alarm_id_t btn_a_alm;
static alarm_id_t btn_b_alm;
static alarm_id_t joy_btn_alm;

uint16_t pct_a;
uint16_t pct_b;
uint16_t pct_alm_a = 30;
uint16_t pct_alm_b = 30;
uint8_t alm_a = NORMAL;
uint8_t alm_b = NORMAL;

// Variável para controle do display
ssd1306_t ssd;

void init_btns() {
    // Cria máscara com os valores dos 3 botões para inicialização
    uint btnMask = (0x01 << BUTTON_A) | (0x01 << BUTTON_B) | (0x01 << JOYSTICK_BTN);
    gpio_init_mask(btnMask);
    gpio_set_dir_in_masked(btnMask);
    gpio_pull_up(BUTTON_A);
    gpio_pull_up(BUTTON_B);
    gpio_pull_up(JOYSTICK_BTN);

    // Inicializa as interrupções para os botão A e botão do Joystick
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &debounce);
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &debounce);
    gpio_set_irq_enabled_with_callback(JOYSTICK_BTN, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &debounce);
}

void init_display() {
    i2c_init(I2C_PORT, SSD1306_FREQ);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
    gpio_pull_up(I2C_SDA); // Pull up the data line
    gpio_pull_up(I2C_SCL); // Pull up the clock line
    
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, SSD1306_ADDR, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);

    // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);
}

void init_joystick_adc() {
    // Inicia o ADC
    adc_init();

    // Configura os pinos do Joystick como ADC
    adc_gpio_init(JOYSTICK_X);
    adc_gpio_init(JOYSTICK_Y);
}

uint64_t handle_btn_a(alarm_id_t id, void *user_data) {
    if(alm_a == ALARM) alm_a = CLEARED;
    else pct_alm_a = pct_a;
}

uint64_t handle_btn_b(alarm_id_t id, void *user_data) {
    if(alm_b == ALARM) alm_b = CLEARED;
    else pct_alm_b = pct_b;
}

uint64_t handle_joy_btn(alarm_id_t id, void *user_data) {

}

static void debounce(uint gpio, uint32_t events) {
    if(gpio == BUTTON_A) {
        cancel_alarm(btn_a_alm);
        if(events == 0x04) btn_a_alm = add_alarm_in_ms(DEBOUNCE_MS, handle_btn_a, NULL, false);
    }
    if(gpio == BUTTON_B) {
        cancel_alarm(btn_b_alm);
        if(events == 0x04) btn_b_alm = add_alarm_in_ms(DEBOUNCE_MS, handle_btn_b, NULL, false);
    }
    else if(gpio == JOYSTICK_BTN){
        cancel_alarm(joy_btn_alm);
        if(events == 0x04) joy_btn_alm = add_alarm_in_ms(DEBOUNCE_MS, handle_joy_btn, NULL, false);
    }
}

void draw_tank(uint16_t left, uint16_t pct, uint16_t pct_alm) {
    ssd1306_rect(&ssd, 0, left, 12, 62, true, false);

    uint16_t top = 60 - 5.8f * pct / 10;
    uint16_t width = 60 - top;
    ssd1306_rect(&ssd, top, left + 2, 8, width, true, true);

    uint16_t pos_alm = 60 - 5.8f * pct_alm / 10;
    ssd1306_hline(&ssd, left - 2, left + 1, pos_alm, true);
    ssd1306_hline(&ssd, left + 2, left + 9, pos_alm, pos_alm <= top);
    ssd1306_hline(&ssd, left + 10, left + 13, pos_alm, true);
}

void update_display(){
    
    // Limpa o display e renderiza o retangulo mais externo (borda fina)
    ssd1306_fill(&ssd, false);

    draw_tank(100, pct_a, pct_alm_a);
    draw_tank(115, pct_b, pct_alm_b);

    ssd1306_send_data(&ssd); // Atualiza o display
}

int main()
{
    stdio_init_all();

    init_btns();
    init_display();
    init_joystick_adc();

    while (true) {
        // Lê os valores do joystick
        adc_select_input(0);
        uint16_t tank_a = adc_read();
        adc_select_input(1);
        uint16_t tank_b = adc_read();

        pct_a = 100 * tank_a / 4096;
        if(pct_a >= pct_alm_a) alm_a = NORMAL;
        else if(alm_a == NORMAL) alm_a = ALARM;
        
        pct_b = 100 * tank_b / 4096;
        if(pct_b >= pct_alm_b) alm_b = NORMAL;
        else if(alm_b == NORMAL) alm_b = ALARM;

        update_display();
        sleep_ms(100);
    }
}
