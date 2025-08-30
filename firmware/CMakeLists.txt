
# Pull in Raspberry Pi Pico SDK (must be before project)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(mikuPixel
  mikuPixel.cpp
  NeoPixelBuffer.cpp
  Miku.cpp
  configService.cpp
  deviceConfig.cpp
  dhcpserver/dhcpserver.c
  dnsserver/dnsserver.c
  mqttClient.cpp
  scheduler.cpp
  serviceStatus.cpp
  wifiConnection.cpp
  wifiScanner.cpp
  webServer.cpp
  blockStorage.cpp
  AnimationController.cpp
  LightController.cpp
  AnimationRunner.cpp
  PatternEditor.cpp
  PatternList.cpp
  animations/MarqueeAnimation.cpp
  animations/MikuSweepAnimation.cpp
  animations/PatternSequenceAnimation.cpp
  animations/PixelMapperAnimation.cpp
  animations/PulsingMikuAnimation.cpp
  animations/TrainAnimation.cpp
  animations/WelcomeAnimation.cpp
  )

pico_set_program_name(mikuPixel "mikuPixel")
pico_set_program_version(mikuPixel "0.1")

pico_enable_stdio_uart(mikuPixel 0)
pico_enable_stdio_usb(mikuPixel 1)

pico_generate_pio_header(mikuPixel ${CMAKE_CURRENT_LIST_DIR}/neopixel.pio)

# Add the standard library to the build
target_link_libraries(
  mikuPixel
  
  pico_stdlib
  hardware_pwm
  hardware_pio
  hardware_dma
  pico_multicore
  pico_cyw43_arch_lwip_poll
  pico_lwip_http
  pico_lwip_mqtt
  pico_flash
  )

# Add the standard include files to the build
target_include_directories(mikuPixel PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts or any other standard includes, if required
)

pico_add_extra_outputs(mikuPixel)

