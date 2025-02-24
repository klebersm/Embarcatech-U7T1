#include "U7T1.h"

struct repeating_timer timer;
bool beep = false;
bool replace_char = false;

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


void initBuzzerPwm() {
    // Seta o pino pra PWM e busca o slice correspondente
    gpio_set_function(BUZZER, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(BUZZER);
  
    // Habilita o slice para o buzzer
    pwm_clear_irq(slice_num);
    pwm_set_irq_enabled(slice_num, true);
  
    // Inicializa a configuração padrão e seta clock e divisor    
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 4.f);
    // Inicializa o slice
    pwm_init(slice_num, &config, true);
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

void draw_tank(uint16_t top, uint16_t pct, uint16_t pct_alm) {
    ssd1306_rect(&ssd, top, 1, 125, 18, true, false);

    uint16_t width = 123 * pct / 100;
    ssd1306_rect(&ssd, top + 2, 3, width, 14, true, true);

    uint16_t pos_alm = 123 * pct_alm / 100;
    ssd1306_vline(&ssd, pos_alm, top - 2, top + 1, true);
    ssd1306_vline(&ssd, pos_alm, top + 2, top + 16, pos_alm >= width);
    ssd1306_vline(&ssd, pos_alm, top + 16, top + 18, true);

    // ssd1306_hline(&ssd, left + 10, left + 13, pos_alm, true);
}

void update_display(){
    
    // Limpa o display e renderiza o retangulo mais externo (borda fina)
    ssd1306_fill(&ssd, false);
    char value[5];

    draw_tank(10, pct_a, pct_alm_a);
    draw_tank(45, pct_b, pct_alm_b);

    sprintf(value, "%d", pct_a);
    ssd1306_draw_string(&ssd, "TQ A", 0, 0);
    ssd1306_draw_string(&ssd, value, 40, 0);
    ssd1306_draw_string(&ssd, "PCT", 60, 0);
    if(alm_a == NORMAL) ssd1306_draw_string(&ssd, "NRML", 90, 0);
    if(alm_a == ALARM || alm_a == CLEARED) ssd1306_draw_string(&ssd, "ALM", 90, 0);
    if(alm_a == CLEARED) ssd1306_hline(&ssd, 88, 115, 3, true);
    
    sprintf(value, "%d", pct_b);
    ssd1306_draw_string(&ssd, "TQ B", 0, 35);
    ssd1306_draw_string(&ssd, value, 40, 35);
    ssd1306_draw_string(&ssd, "PCT", 60, 35);
    if(alm_b == NORMAL) ssd1306_draw_string(&ssd, "NRML", 90, 35);
    if(alm_b == ALARM || alm_b == CLEARED) ssd1306_draw_string(&ssd, "ALM", 90, 35);
    if(alm_b == CLEARED) ssd1306_hline(&ssd, 88, 115, 38, true);

    ssd1306_send_data(&ssd); // Atualiza o display
}

bool repeat_callback(struct repeating_timer *t) {
    bool show_a = alm_a != NORMAL;
    bool show_b = alm_b != NORMAL;
    
    if(show_a && (!show_b || replace_char)) {
        uint32_t color = alm_a == ALARM ? urgb_u32(150, 0, 0) : urgb_u32(150,50,0);
        renderMatrix(CHAR_A, color);
    }
    if(show_b && (!show_a || !replace_char)) {
        uint32_t color = alm_b == ALARM ? urgb_u32(150, 0, 0) : urgb_u32(150,50,0);
        renderMatrix(CHAR_B, color);
    }
    if(!show_a && !show_b) renderMatrix(OFF, 0);

    if(alm_a == ALARM || alm_b == ALARM) {
        uint16_t level = beep ? 2000 : 0;     
        pwm_set_gpio_level(BUZZER, level);        
        beep = !beep;
    }
    else { 
        pwm_set_gpio_level(BUZZER, 0);
        beep = false;
    }
    
    replace_char = !replace_char;
    return true;
}

int main()
{
    stdio_init_all();

    init_btns();
    init_display();
    init_joystick_adc();
    
    initBuzzerPwm();
    // Inicializa a matriz de leds
    initNeoPixel();

    // Inicia uma repetição a cada 1 segundo para gerenciar alarmes sonoros e visuais
    add_repeating_timer_ms(500, repeat_callback, NULL, &timer);

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
