<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="es" sourcelanguage="en">
<context>
    <name>CpuMonitor</name>
    <message>
        <location filename="../src/backend/monitor/cpumonitor.cpp" line="+339"/>
        <source>CPU temperature is being read from system sensors.</source>
        <translation>La temperatura de la CPU se lee desde los sensores del sistema.</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>CPU temperature sensor is not exposed by the kernel.</source>
        <translation>El kernel no expone el sensor de temperatura de la CPU.</translation>
    </message>
</context>
<context>
    <name>DriverPage</name>
    <message>
        <location filename="../src/qml/pages/DriverPage.qml" line="+20"/>
        <location line="+298"/>
        <location line="+926"/>
        <source>Ready</source>
        <translation>Listo</translation>
    </message>
    <message>
        <location line="-938"/>
        <source>Update</source>
        <translation>Actualización</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Package</source>
        <translation>Paquete</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Kernel</source>
        <translation>Kernel</translation>
    </message>
    <message>
        <location line="+20"/>
        <source>%1 completed: %2</source>
        <translation>%1 completado: %2</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>%1 failed: %2</source>
        <translation>%1 falló: %2</translation>
    </message>
    <message>
        <location line="+8"/>
        <location line="+1"/>
        <source>Cancel requested. Waiting for the active command to stop safely...</source>
        <translation>Cancelación solicitada. Esperando a que el comando activo se detenga de forma segura...</translation>
    </message>
    <message>
        <location line="+5"/>
        <location line="+1"/>
        <source>Restart request failed. Please restart the computer manually.</source>
        <translation>No se pudo solicitar el reinicio. Reinicie el equipo manualmente.</translation>
    </message>
    <message>
        <location line="+4"/>
        <location line="+1"/>
        <source>Restart requested.</source>
        <translation>Reinicio solicitado.</translation>
    </message>
    <message>
        <location line="+24"/>
        <source>NVIDIA driver cleanup completed. Restart recommended.</source>
        <translation>Limpieza del controlador NVIDIA completada. Se recomienda reiniciar.</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Refreshing driver status shown on this page...</source>
        <translation>Actualizando el estado del controlador mostrado en esta página...</translation>
    </message>
    <message>
        <location line="+43"/>
        <source>Official Package • akmod-nvidia &amp; CUDA libraries</source>
        <translation>Paquete oficial • akmod-nvidia y bibliotecas CUDA</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Installs NVIDIA&apos;s official proprietary binary driver stack. This stack delivers full hardware feature support including DLSS, CUDA acceleration, NVENC hardware encoding, OptiX, and Ray Tracing.</source>
        <translation>Instala la pila oficial de controladores binarios propietarios de NVIDIA. Proporciona soporte completo de hardware incluyendo DLSS, aceleración CUDA, codificación NVENC, OptiX y Ray Tracing.</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Downloads and installs akmod-nvidia, xorg-x11-drv-nvidia, and core libraries.</source>
        <translation>Descarga e instala akmod-nvidia, xorg-x11-drv-nvidia y bibliotecas principales.</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Compiles the proprietary kernel module against your active Linux kernel (%1).</source>
        <translation>Compila el módulo de kernel propietario para su kernel de Linux activo (%1).</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Configures kernel parameters (nvidia-drm.modeset=1) and updates initramfs.</source>
        <translation>Configura parámetros del kernel (nvidia-drm.modeset=1) y actualiza initramfs.</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>A system reboot is required after installation to activate the kernel driver.</source>
        <translation>Se requiere reiniciar el sistema tras la instalación para activar el controlador del kernel.</translation>
    </message>
    <message>
        <location line="+16"/>
        <source>Hardware Requirement: Turing (RTX 2000 / GTX 1600) or newer GPU architecture.</source>
        <translation>Requisito de hardware: Arquitectura de GPU Turing (RTX 2000 / GTX 1600) o más reciente.</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Compiles akmod-nvidia-open module directly with standard Linux kernel interfaces.</source>
        <translation>Compila el módulo akmod-nvidia-open directamente con interfaces estándar del kernel de Linux.</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Older architectures (Pascal/Maxwell/GTX 1000 and earlier) are not supported by the open kernel module.</source>
        <translation>Las arquitecturas antiguas (Pascal/Maxwell/GTX 1000 y anteriores) no son compatibles con el módulo de kernel abierto.</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Deep Clean &amp; Module Purge</source>
        <translation>Limpieza profunda y purga de módulos</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Prepares system for a clean, conflict-free driver installation or stack switch.</source>
        <translation>Prepara el sistema para una instalación limpia y sin conflictos o cambio de pila de controladores.</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Does not delete personal files or desktop settings. Restart is recommended after cleanup.</source>
        <translation>No elimina archivos personales ni configuraciones del escritorio. Se recomienda reiniciar tras la limpieza.</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Run Deep Clean</source>
        <translation>Ejecutar limpieza profunda</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Rebuild Kernel Modules &amp; Initramfs</source>
        <translation>Reconstruir módulos de kernel e Initramfs</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Akmods Force Recompilation &amp; Dracut Image Regeneration</source>
        <translation>Forzar recompilación de Akmods y regeneración de Dracut</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Forces a complete recompilation of NVIDIA kernel modules against the currently running Linux kernel and updates the early boot ramdisk (initramfs).</source>
        <translation>Fuerza una recompilación completa de módulos de NVIDIA para el kernel actual y actualiza el disco RAM de arranque temprano (initramfs).</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Executes &apos;akmods --force&apos; to recompile the driver for kernel: %1.</source>
        <translation>Ejecuta &apos;akmods --force&apos; para recompilar el controlador para el kernel: %1.</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Executes &apos;dracut -f&apos; to package the compiled modules into the bootloader image.</source>
        <translation>Ejecuta &apos;dracut -f&apos; para empaquetar los módulos compilados en la imagen de arranque.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>This operation may take 30 to 90 seconds depending on system CPU speed.</source>
        <translation>Esta operación puede tardar de 30 a 90 segundos según la velocidad de la CPU.</translation>
    </message>
    <message>
        <location line="+53"/>
        <source>Latest available: %1</source>
        <translation>Más reciente disponible: %1</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Driver catalog loaded</source>
        <translation>Catálogo de controladores cargado</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Driver scan pending</source>
        <translation>Escaneo de controladores pendiente</translation>
    </message>
    <message>
        <location line="+22"/>
        <source>New version available: %1</source>
        <translation>Nueva version disponible: %1</translation>
    </message>
    <message>
        <location line="+81"/>
        <source>Restart required — the installed NVIDIA driver will be active after reboot.</source>
        <translation>Se requiere reiniciar — el controlador NVIDIA instalado se activará después del reinicio.</translation>
    </message>
    <message>
        <location line="+191"/>
        <source>UEFI Secure Boot is active.
Third-party akmod modules require MOK signing.</source>
        <translation>UEFI Secure Boot está activo.
Los módulos akmod de terceros requieren firma MOK.</translation>
    </message>
    <message>
        <location line="+53"/>
        <source>NVIDIA driver controls are disabled because no NVIDIA GPU is detected. CPU, memory, and non-NVIDIA hardware monitoring remain available.</source>
        <translation>Los controles del controlador NVIDIA están deshabilitados porque no se detectó ninguna GPU NVIDIA. La monitorización de CPU, memoria y hardware no NVIDIA sigue disponible.</translation>
    </message>
    <message>
        <location line="-261"/>
        <source>An NVIDIA GPU or NVIDIA passthrough device is required.</source>
        <translation>Se requiere una GPU NVIDIA o un dispositivo de paso NVIDIA (passthrough).</translation>
    </message>
    <message>
        <location line="+260"/>
        <source>Manage proprietary NVIDIA modules and NVIDIA Open Kernel Modules. Switching modules requires Deep Clean first.</source>
        <translation>Administre los módulos privativos de NVIDIA y los módulos abiertos del kernel NVIDIA. El cambio de módulos requiere Limpieza profunda primero.</translation>
    </message>
    <message>
        <location line="+19"/>
        <source>Managed installation and updates are available only in a Wayland session. Switch sessions, then refresh this page.</source>
        <translation>La instalación y las actualizaciones administradas solo están disponibles en una sesión Wayland. Cambie de sesión y actualice esta página.</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>NVIDIA Proprietary Module</source>
        <translation>Módulo privativo de NVIDIA</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Deep Clean is required before switching from NVIDIA Open Kernel Modules to the proprietary module.</source>
        <translation>Se requiere Limpieza profunda antes de cambiar de los módulos abiertos del kernel NVIDIA al módulo privativo.</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Install the proprietary NVIDIA kernel module (akmod-nvidia).</source>
        <translation>Instale el módulo privativo del kernel NVIDIA (akmod-nvidia).</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>akmod-nvidia-open</source>
        <translation>akmod-nvidia-open</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Deep Clean is required before switching from the proprietary module to NVIDIA Open Kernel Modules.</source>
        <translation>Se requiere Limpieza profunda antes de cambiar del módulo privativo a los módulos abiertos del kernel NVIDIA.</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Install NVIDIA Open Kernel Modules (akmod-nvidia-open).</source>
        <translation>Instale los módulos abiertos del kernel NVIDIA (akmod-nvidia-open).</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Remove NVIDIA packages and clear DNF cache</source>
        <translation>Eliminar paquetes NVIDIA y limpiar la caché de DNF</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>An installed NVIDIA driver is required for cleanup.</source>
        <translation>Se requiere un controlador NVIDIA instalado para realizar la limpieza.</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Remove NVIDIA packages and clear cached repository metadata.</source>
        <translation>Elimina los paquetes de NVIDIA y borra los metadatos en caché del repositorio.</translation>
    </message>
    <message>
        <location line="+109"/>
        <source>Collapse</source>
        <translation>Contraer</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>View log</source>
        <translation>Ver registro</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Operation is running. Open the log to follow progress.</source>
        <translation>La operación está en curso. Abra el registro para seguir el progreso.</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Latest activity is available. Open the log to review it.</source>
        <translation>La actividad más reciente está disponible. Abra el registro para revisarla.</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>No active driver operation.</source>
        <translation>No hay ninguna operación activa del controlador.</translation>
    </message>
    <message>
        <location line="+413"/>
        <source>NVIDIA Open Kernel Modules are currently detected. Run Deep Clean before installing the proprietary NVIDIA module.</source>
        <translation>Actualmente se detectan los módulos abiertos del kernel NVIDIA. Ejecute Limpieza profunda antes de instalar el módulo privativo de NVIDIA.</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>The proprietary NVIDIA module is currently detected. Run Deep Clean before installing NVIDIA Open Kernel Modules.</source>
        <translation>Actualmente se detecta el módulo privativo de NVIDIA. Ejecute Limpieza profunda antes de instalar los módulos abiertos del kernel NVIDIA.</translation>
    </message>
    <message>
        <location line="+169"/>
        <source>Proceed</source>
        <translation>Continuar</translation>
    </message>
    <message>
        <location line="+247"/>
        <source>Secure Boot MOK Enrollment</source>
        <translation>Registro MOK de Secure Boot</translation>
    </message>
    <message>
        <location line="+42"/>
        <source>Reboot &amp; Intercept</source>
        <translation>Reiniciar e interceptar</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Select &apos;Enroll MOK&apos;</source>
        <translation>Seleccione &apos;Enroll MOK&apos;</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Choose &apos;Enroll MOK&apos; from the menu, select &apos;Continue&apos;, and confirm with &apos;Yes&apos;.</source>
        <translation>Elija &apos;Enroll MOK&apos; en el menú, seleccione &apos;Continue&apos; y confirme con &apos;Yes&apos;.</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Confirm &amp; Reboot</source>
        <translation>Confirmar y reiniciar</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Enter your enrollment password if prompted, then select &apos;Reboot&apos;. Your modules are now permanently trusted.</source>
        <translation>Introduzca su contraseña de registro si se le solicita y seleccione &apos;Reboot&apos;. Sus módulos ahora son de confianza permanente.</translation>
    </message>
    <message>
        <location line="+70"/>
        <source>Got It</source>
        <translation>Entendido</translation>
    </message>
    <message>
        <location line="-1690"/>
        <location line="+1"/>
        <location line="+5"/>
        <location line="+1"/>
        <location line="+4"/>
        <location line="+1"/>
        <location line="+31"/>
        <location line="+203"/>
        <location line="+667"/>
        <source>System</source>
        <translation>Sistema</translation>
    </message>
    <message>
        <location line="-1049"/>
        <source>ACTIVE</source>
        <translation>ACTIVO</translation>
    </message>
    <message>
        <location line="+326"/>
        <location line="+1113"/>
        <source>Installing closed-source NVIDIA driver...</source>
        <translation>Instalando controlador NVIDIA de código cerrado...</translation>
    </message>
    <message>
        <location line="-1027"/>
        <source>Mixed driver state</source>
        <translation>Estado mixto del controlador</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Not detected</source>
        <translation>No detectado</translation>
    </message>
    <message>
        <location line="+96"/>
        <source>Driver</source>
        <translation>Controlador</translation>
    </message>
    <message>
        <location line="+211"/>
        <source>NVIDIA Official Release • Proprietary</source>
        <translation>Lanzamiento oficial de NVIDIA • Propietario</translation>
    </message>
    <message>
        <location line="+3"/>
        <location line="+13"/>
        <source>INSTALLED</source>
        <translation>INSTALADO</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>Akmods &amp; initramfs regeneration</source>
        <translation>Regeneración de Akmods e initramfs</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>An installed NVIDIA driver is required to rebuild modules.</source>
        <translation>Se requiere un controlador NVIDIA instalado para reconstruir los módulos.</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Managed NVIDIA maintenance requires a Wayland session.</source>
        <translation>El mantenimiento administrado de NVIDIA requiere una sesión Wayland.</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Apply the latest compatible package version</source>
        <translation>Aplicar la versión de paquete compatible más reciente</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Install the available NVIDIA driver update.</source>
        <translation>Instalar la actualización disponible del controlador NVIDIA.</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Restart System</source>
        <translation>Reiniciar sistema</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Reboot to activate new driver</source>
        <translation>Reiniciar para activar el nuevo controlador</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>System restart required to load newly installed kernel driver.</source>
        <translation>Se requiere reiniciar el sistema para cargar el controlador de kernel recién instalado.</translation>
    </message>
    <message>
        <location line="+66"/>
        <source>Live</source>
        <translation>En vivo</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Reading</source>
        <translation>Leyendo</translation>
    </message>
    <message>
        <location line="+57"/>
        <source>&gt; Ready. Awaiting driver tasks, updates, or kernel operations...</source>
        <translation>&gt; Listo. Esperando tareas de controlador, actualizaciones u operaciones de kernel...</translation>
    </message>
    <message>
        <location line="+47"/>
        <source>Paused for reading</source>
        <translation>Pausado para lectura</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Copied ✓</source>
        <translation>Copiado ✓</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Copy</source>
        <translation>Copiar</translation>
    </message>
    <message>
        <location line="+18"/>
        <source>Follow</source>
        <translation>Seguir</translation>
    </message>
    <message>
        <location line="+67"/>
        <source>Driver is already up to date.</source>
        <translation>El controlador ya está actualizado.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Driver page status refreshed.</source>
        <translation>Estado de la página del controlador actualizado.</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Driver page status refresh failed.</source>
        <translation>No se pudo actualizar el estado de la página del controlador.</translation>
    </message>
    <message>
        <location line="+54"/>
        <source>Restart Computer</source>
        <translation>Reiniciar equipo</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>A driver operation has completed and the computer must restart before the new graphics stack is active.</source>
        <translation>Una operación del controlador se completó y el equipo debe reiniciarse antes de que la nueva pila gráfica esté activa.</translation>
    </message>
    <message>
        <location line="+29"/>
        <source>Secure Boot active: If prompted on restart, complete the one-time MOK enrollment to authenticate the driver.</source>
        <translation>Secure Boot activo: Si se le solicita al reiniciar, complete el registro MOK para autenticar el controlador.</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>MOK Guide</source>
        <translation>Guía MOK</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>Restart Now</source>
        <translation>Reiniciar ahora</translation>
    </message>
    <message>
        <location line="+42"/>
        <source>Driver Is Already Current</source>
        <translation>El controlador ya está actualizado</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>The installed NVIDIA driver already matches the latest version available from the configured driver sources. Reinstall only if you want to rebuild the driver packages and kernel module.</source>
        <translation>El controlador NVIDIA instalado ya coincide con la versión más reciente disponible en las fuentes de controladores configuradas. Reinstálelo solo si desea reconstruir los paquetes del controlador y el módulo del kernel.</translation>
    </message>
    <message>
        <location line="-232"/>
        <location line="+176"/>
        <location line="+66"/>
        <location line="+69"/>
        <location line="+317"/>
        <source>Cancel</source>
        <translation>Cancelar</translation>
    </message>
    <message>
        <location line="-1449"/>
        <source>NVIDIA proprietary kernel module prepared: %1. Restart required.</source>
        <translation>Módulo privativo del kernel NVIDIA preparado: %1. Es necesario reiniciar.</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>NVIDIA proprietary kernel module prepared. Restart required.</source>
        <translation>Módulo privativo del kernel NVIDIA preparado. Es necesario reiniciar.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>NVIDIA Open Kernel Modules prepared. Restart required.</source>
        <translation>Módulos abiertos del kernel NVIDIA preparados. Es necesario reiniciar.</translation>
    </message>
    <message>
        <location line="+24"/>
        <location line="+575"/>
        <source>Update NVIDIA Driver</source>
        <translation>Actualizar controlador NVIDIA</translation>
    </message>
    <message>
        <location line="-574"/>
        <source>Apply the latest version from configured repositories</source>
        <translation>Aplicar la última versión desde los repositorios configurados</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Updates the installed NVIDIA driver package set, rebuilds its kernel module, and regenerates initramfs.</source>
        <translation>Actualiza el conjunto de paquetes del controlador NVIDIA instalado, recompila su módulo de kernel y regenera el initramfs.</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Applies the latest compatible driver package version.</source>
        <translation>Aplica la versión compatible más reciente del paquete del controlador.</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Rebuilds the active NVIDIA kernel module with akmods.</source>
        <translation>Recompila el módulo activo del kernel NVIDIA con akmods.</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>A restart is required before the updated kernel module is active.</source>
        <translation>Es necesario reiniciar antes de que el módulo del kernel actualizado esté activo.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Secure Boot is enabled. Confirm that a MOK key is enrolled before restarting, or the NVIDIA module may not load.</source>
        <translation>Secure Boot está habilitado. Confirme que se haya registrado una clave MOK antes de reiniciar, o el módulo de NVIDIA podría no cargarse.</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>The display session must be Wayland for this managed update flow.</source>
        <translation>La sesión de pantalla debe ser Wayland para este flujo de actualización administrado.</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Update Driver</source>
        <translation>Actualizar controlador</translation>
    </message>
    <message>
        <location line="+16"/>
        <source>NVIDIA Proprietary Kernel Module (akmod-nvidia)</source>
        <translation>Módulo privativo del kernel NVIDIA (akmod-nvidia)</translation>
    </message>
    <message>
        <location line="+10"/>
        <location line="+22"/>
        <source>Secure Boot is enabled. Enroll the akmods MOK key before restarting, or the NVIDIA module may not load.</source>
        <translation>Secure Boot está habilitado. Registre la clave MOK de akmods antes de reiniciar, o el módulo de NVIDIA podría no cargarse.</translation>
    </message>
    <message>
        <location line="-20"/>
        <source>Install Proprietary Module</source>
        <translation>Instalar módulo privativo</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>NVIDIA Open Kernel Modules (akmod-nvidia-open)</source>
        <translation>Módulos abiertos del kernel NVIDIA (akmod-nvidia-open)</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>NVIDIA driver with open kernel modules</source>
        <translation>Controlador NVIDIA con módulos abiertos del kernel</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Installs NVIDIA Open Kernel Modules. This is not a full community graphics stack: NVIDIA userspace components remain part of the installation.</source>
        <translation>Instala los módulos abiertos del kernel NVIDIA. No se trata de una pila gráfica completa de la comunidad: los componentes de espacio de usuario (userspace) de NVIDIA permanecen como parte de la instalación.</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Updates bootloader image (dracut initramfs) with NVIDIA Open Kernel Modules.</source>
        <translation>Actualiza la imagen del cargador de arranque (dracut initramfs) con los módulos abiertos del kernel NVIDIA.</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Install Open Kernel Modules</source>
        <translation>Instalar módulos abiertos del kernel</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Remove NVIDIA packages and clear cached metadata</source>
        <translation>Eliminar paquetes NVIDIA y limpiar metadatos en caché</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Removes installed NVIDIA driver packages and clears DNF&apos;s cached metadata so a later installation starts from a clean package state.</source>
        <translation>Elimina los paquetes del controlador NVIDIA instalados y borra los metadatos en caché de DNF para que una instalación posterior comience desde un estado de paquetes limpio.</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Removes akmod-nvidia, akmod-nvidia-open, NVIDIA Xorg packages, and nvidia-settings.</source>
        <translation>Elimina akmod-nvidia, akmod-nvidia-open, los paquetes Xorg de NVIDIA y nvidia-settings.</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Runs &apos;dnf clean all&apos; to remove cached repository metadata.</source>
        <translation>Ejecuta &apos;dnf clean all&apos; para eliminar los metadatos en caché de los repositorios.</translation>
    </message>
    <message>
        <location line="+16"/>
        <source>Repairs NVIDIA module build failures that can follow Linux kernel updates.</source>
        <translation>Repara los fallos de compilación de los módulos de NVIDIA que pueden surgir tras actualizaciones del kernel Linux.</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Installing NVIDIA Open Kernel Modules...</source>
        <translation>Instalando módulos abiertos del kernel NVIDIA...</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Updating NVIDIA driver...</source>
        <translation>Actualizando el controlador NVIDIA...</translation>
    </message>
    <message>
        <location line="+45"/>
        <source>VM detected. NVIDIA passthrough required.</source>
        <translation>Máquina virtual detectada. Se requiere NVIDIA passthrough.</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>No NVIDIA GPU or driver.</source>
        <translation>No se detectó GPU NVIDIA ni controlador.</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Refreshing status...</source>
        <translation>Actualizando estado...</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Restart may be required.</source>
        <translation>Es posible que sea necesario reiniciar.</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Up to date.</source>
        <translation>Actualizado.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Installed.</source>
        <translation>Instalado.</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Latest: %1</source>
        <translation>Más reciente: %1</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Catalog loaded.</source>
        <translation>Catálogo cargado.</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Checking updates...</source>
        <translation>Buscando actualizaciones...</translation>
    </message>
    <message>
        <location line="+37"/>
        <source>NVIDIA Proprietary Kernel Module</source>
        <translation>Módulo privativo del kernel NVIDIA</translation>
    </message>
    <message>
        <location line="+2"/>
        <location line="+322"/>
        <source>NVIDIA Open Kernel Modules</source>
        <translation>Módulos abiertos del kernel NVIDIA</translation>
    </message>
    <message>
        <location line="-314"/>
        <source>State unreadable.</source>
        <translation>Estado no legible.</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Signing may be required.</source>
        <translation>Puede requerirse firma.</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>No signing required.</source>
        <translation>No se requiere firma.</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Managed NVIDIA installation and updates require a Wayland session.</source>
        <translation>La instalación y las actualizaciones administradas de NVIDIA requieren una sesión Wayland.</translation>
    </message>
    <message>
        <location line="+52"/>
        <source>VM display. Use NVIDIA passthrough.</source>
        <translation>Pantalla VM. Utilice NVIDIA passthrough.</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>NVIDIA hardware required.</source>
        <translation>Se requiere hardware NVIDIA.</translation>
    </message>
    <message>
        <location line="+48"/>
        <source>UPDATE AVAILABLE</source>
        <translation>ACTUALIZACIÓN DISPONIBLE</translation>
    </message>
    <message>
        <location line="+83"/>
        <source>MOK GUIDE ↗</source>
        <translation>GUÍA MOK ↗</translation>
    </message>
    <message>
        <location line="+919"/>
        <source>I have completed the required akmods MOK key enrollment and understand that the NVIDIA module will not load without it.</source>
        <translation>He completado el registro requerido de la clave MOK de akmods y entiendo que el módulo de NVIDIA no se cargará sin él.</translation>
    </message>
    <message>
        <location line="+180"/>
        <source>Required before restarting after an NVIDIA driver installation</source>
        <translation>Requerido antes de reiniciar después de una instalación del controlador NVIDIA</translation>
    </message>
    <message>
        <location line="+35"/>
        <source>Before installation, generate and import the akmods key using your Fedora Secure Boot procedure. Then restart and enter Shim UEFI Key Management when prompted.</source>
        <translation>Antes de la instalación, genere e importe la clave akmods siguiendo el procedimiento de Secure Boot de Fedora. Luego reinicie y entre a Administración de claves Shim UEFI cuando se le solicite.</translation>
    </message>
    <message>
        <location line="-1081"/>
        <source>Driver Stack</source>
        <translation>Pila de controladores</translation>
    </message>
    <message>
        <location line="-380"/>
        <location line="+471"/>
        <source>Rebuild Modules</source>
        <translation>Reconstruir módulos</translation>
    </message>
    <message>
        <location line="-452"/>
        <source>Rebuilding kernel modules &amp; initramfs...</source>
        <translation>Reconstruyendo módulos de kernel e initramfs...</translation>
    </message>
    <message>
        <location line="+458"/>
        <source>Force-rebuilds akmod kernel modules and regenerates initramfs after kernel updates.</source>
        <translation>Fuerza la reconstrucción de módulos akmod y regenera initramfs tras actualizaciones del kernel.</translation>
    </message>
    <message>
        <location line="+191"/>
        <source>Command output is being captured.</source>
        <translation>Se está capturando la salida del comando.</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Following output</source>
        <translation>Salida continua</translation>
    </message>
    <message>
        <location line="+280"/>
        <source>Reinstall Anyway</source>
        <translation>Reinstalar de todos modos</translation>
    </message>
    <message>
        <location line="+43"/>
        <source>Deep Clean Required</source>
        <translation>Limpieza Profunda Requerida</translation>
    </message>
    <message>
        <location line="+68"/>
        <source>NVIDIA License Review</source>
        <translation>Revisión de licencia NVIDIA</translation>
    </message>
    <message>
        <location line="+18"/>
        <source>Close</source>
        <translation>Cerrar</translation>
    </message>
    <message>
        <location line="+63"/>
        <source>Reject</source>
        <translation>Rechazar</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Accept</source>
        <translation>Aceptar</translation>
    </message>
    <message>
        <location line="-968"/>
        <source>GPU</source>
        <translation>GPU</translation>
    </message>
    <message>
        <location line="-166"/>
        <location line="+1009"/>
        <source>Cleaning NVIDIA artifacts...</source>
        <translation>Limpiando restos de NVIDIA...</translation>
    </message>
    <message>
        <location line="-495"/>
        <source>Activity</source>
        <translation>Actividad</translation>
    </message>
    <message>
        <location line="+179"/>
        <source>Clear</source>
        <translation>Limpiar</translation>
    </message>
    <message>
        <location line="-885"/>
        <source>General</source>
        <translation>General</translation>
    </message>
    <message>
        <location line="+270"/>
        <source>Closed-source NVIDIA driver installation requires reviewing and accepting the NVIDIA license terms before ro-Control can start the closed-source install workflow.</source>
        <translation>La instalación del controlador NVIDIA de código cerrado requiere revisar y aceptar los términos de licencia de NVIDIA antes de que ro-Control pueda iniciar el flujo de instalación de código cerrado.</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>Checking official NVIDIA driver sources...</source>
        <translation>Comprobando fuentes oficiales de controladores NVIDIA...</translation>
    </message>
    <message>
        <location line="-51"/>
        <source>No NVIDIA GPU</source>
        <translation>Sin GPU NVIDIA</translation>
    </message>
    <message>
        <location line="+228"/>
        <source>Secure Boot</source>
        <translation>Arranque seguro</translation>
    </message>
    <message>
        <location line="+47"/>
        <source>Enabled</source>
        <translation>Activado</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Disabled</source>
        <translation>Desactivado</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Unknown</source>
        <translation>Desconocido</translation>
    </message>
    <message>
        <location line="+53"/>
        <source>Rescan and check updates</source>
        <translation>Volver a escanear y buscar actualizaciones</translation>
    </message>
    <message>
        <location line="-382"/>
        <location line="+4"/>
        <location line="+4"/>
        <location line="+22"/>
        <location line="+684"/>
        <location line="+1"/>
        <location line="+4"/>
        <location line="+1"/>
        <location line="+2"/>
        <location line="+5"/>
        <location line="+1"/>
        <location line="+2"/>
        <location line="+283"/>
        <location line="+130"/>
        <source>Installer</source>
        <translation>Instalador</translation>
    </message>
    <message>
        <location line="-1131"/>
        <location line="+91"/>
        <location line="+636"/>
        <location line="+1"/>
        <location line="+5"/>
        <location line="+2"/>
        <location line="+1"/>
        <location line="+2"/>
        <location line="+9"/>
        <location line="+1"/>
        <location line="+4"/>
        <source>Updater</source>
        <translation>Actualizador</translation>
    </message>
    <message>
        <location line="-315"/>
        <location line="+558"/>
        <source>Deep Clean</source>
        <translation>Limpieza profunda</translation>
    </message>
</context>
<context>
    <name>FanController</name>
    <message>
        <location filename="../src/backend/fan/fancontroller.cpp" line="+233"/>
        <source>Hardware fan scan complete: %1 channel(s) detected.</source>
        <translation>Escaneo de ventiladores de hardware completado: %1 canal(es) detectado(s).</translation>
    </message>
    <message>
        <location line="+79"/>
        <source>Polkit (pkexec) is not available to configure Coolbits.</source>
        <translation>Polkit (pkexec) no está disponible para configurar Coolbits.</translation>
    </message>
    <message>
        <location line="+23"/>
        <source>Coolbits enabled successfully! A session restart or reboot is required to activate manual fan control.</source>
        <translation>¡Coolbits activado con éxito! Se requiere reiniciar la sesión o el equipo para habilitar el control manual del ventilador.</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Failed to enable Coolbits: %1</source>
        <translation>No se pudo habilitar Coolbits: %1</translation>
    </message>
    <message>
        <location line="+575"/>
        <source>Automatic Mode: NVIDIA telemetry active.</source>
        <translation>Modo automático: Telemetría de NVIDIA activa.</translation>
    </message>
    <message>
        <location line="+121"/>
        <source>Controllable</source>
        <translation>Controlable</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>0 RPM (Silent)</source>
        <translation>0 RPM (Silencioso)</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Active (Auto)</source>
        <translation>Activo (Automático)</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Direct hardware fan control active via NV-CONTROL.</source>
        <translation>Control directo de hardware de ventilador activo mediante NV-CONTROL.</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>GPU is in 0 RPM silent mode (temperature &lt; 50°C). Fans automatically spin up under load.</source>
        <translation>La GPU está en modo silencioso de 0 RPM (temperatura &lt; 50°C). Los ventiladores giran automáticamente bajo carga.</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Automatic VBIOS cooling curve active.</source>
        <translation>Curva de refrigeración automática de VBIOS activa.</translation>
    </message>
    <message>
        <location line="+176"/>
        <source>Hardware BIOS thermal curve active with dynamic acoustic regulation.</source>
        <translation>Curva térmica de BIOS de hardware activa con regulación acústica dinámica.</translation>
    </message>
    <message>
        <location line="+81"/>
        <source>Motherboard chassis airflow management curve active.</source>
        <translation>Curva de gestión de flujo de aire del chasis activa.</translation>
    </message>
    <message>
        <location line="+46"/>
        <source>Hardware-managed</source>
        <translation>Administrado por hardware</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Live RPM telemetry is available; this channel is managed by system firmware.</source>
        <translation>Telemetría RPM en vivo disponible; este canal es administrado por el firmware del sistema.</translation>
    </message>
    <message>
        <location line="+194"/>
        <source>Safety Override Active: GPU is hot (%1°C &gt;= %2°C). Fan forced to 100%.</source>
        <translation>Protección de seguridad activa: la GPU está caliente (%1°C &gt;= %2°C). Ventilador forzado al 100%.</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Automatic Mode: Managed by VBIOS and driver.</source>
        <translation>Modo automático: Administrado por VBIOS y el controlador.</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Silent Profile Active (%1% @ %2°C).</source>
        <translation>Perfil silencioso activo (%1% a %2°C).</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Balanced Optimization Active (%1% @ %2°C).</source>
        <translation>Optimización equilibrada activa (%1% a %2°C).</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Performance Profile Active (%1% @ %2°C).</source>
        <translation>Perfil de rendimiento activo (%1% a %2°C).</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Manual Fan Speed Locked at %1%.</source>
        <translation>Velocidad manual del ventilador bloqueada al %1%.</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Custom Curve Active (%1% @ %2°C).</source>
        <translation>Curva personalizada activa (%1% a %2°C).</translation>
    </message>
    <message>
        <location line="+104"/>
        <source>NVIDIA fan control rejected by driver: Coolbits option is required in Xorg configuration.</source>
        <translation>El controlador rechazó el control del ventilador NVIDIA: se requiere la opción Coolbits en la configuración de Xorg.</translation>
    </message>
    <message>
        <location line="+79"/>
        <source>Direct GPU fan control is not available.</source>
        <translation>El control directo del ventilador de la GPU no está disponible.</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>This fan is monitored by firmware and cannot be controlled by ro-Control.</source>
        <translation>Este ventilador es monitoreado por el firmware y no puede ser controlado por ro-Control.</translation>
    </message>
</context>
<context>
    <name>FanPage</name>
    <message>
        <location filename="../src/qml/pages/FanPage.qml" line="+186"/>
        <location line="+562"/>
        <source>Silent</source>
        <translation>Silencioso</translation>
    </message>
    <message>
        <location line="-561"/>
        <location line="+562"/>
        <location line="+246"/>
        <source>Balanced</source>
        <translation>Equilibrado</translation>
    </message>
    <message>
        <location line="-807"/>
        <location line="+562"/>
        <source>Performance</source>
        <translation>Rendimiento</translation>
    </message>
    <message>
        <location line="-561"/>
        <location line="+562"/>
        <source>Manual</source>
        <translation>Manual</translation>
    </message>
    <message>
        <location line="-561"/>
        <location line="+562"/>
        <source>Custom</source>
        <translation>Personalizado</translation>
    </message>
    <message>
        <location line="-560"/>
        <location line="+555"/>
        <source>Auto</source>
        <translation>Automático</translation>
    </message>
    <message>
        <location line="-548"/>
        <source>Acoustic priority profile. Keeps fans quiet and delays ramp-up for quiet operation.</source>
        <translation>Perfil de prioridad acústica. Mantiene los ventiladores silenciosos y retrasa la aceleración para un funcionamiento silencioso.</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Custom temperature-to-speed curve with hysteresis and response smoothing.</source>
        <translation>Curva personalizada de temperatura a velocidad con histéresis y suavizado de respuesta.</translation>
    </message>
    <message>
        <location line="+60"/>
        <source>Cooling Channels (%1)</source>
        <translation>Canales de refrigeración (%1)</translation>
    </message>
    <message>
        <location line="+191"/>
        <source>Open fan settings</source>
        <translation>Abrir ajustes del ventilador</translation>
    </message>
    <message>
        <location line="+19"/>
        <source>SPEED</source>
        <translation>VELOCIDAD</translation>
    </message>
    <message>
        <location line="+65"/>
        <source>TEMPERATURE</source>
        <translation>TEMPERATURA</translation>
    </message>
    <message>
        <location line="+82"/>
        <source>Slot %1</source>
        <translation>Ranura %1</translation>
    </message>
    <message>
        <location line="+55"/>
        <source>Only one RPM channel is exposed by Linux. CPU and chassis fans will appear automatically when the motherboard firmware or kernel sensor driver publishes their RPM telemetry.</source>
        <translation>Linux expone solo un canal de RPM. Los ventiladores de CPU y chasis aparecerán automáticamente cuando el firmware de la placa base o el controlador de sensores del kernel publiquen su telemetría.</translation>
    </message>
    <message>
        <location line="+68"/>
        <source>Hardware dynamic</source>
        <translation>Dinámico por hardware</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Zero-dB quiet</source>
        <translation>Silencioso Zero-dB</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Optimized blend</source>
        <translation>Equilibrio optimizado</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Maximum airflow</source>
        <translation>Flujo de aire máximo</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Locked speed</source>
        <translation>Velocidad fija</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>User curve</source>
        <translation>Curva personalizada</translation>
    </message>
    <message>
        <location line="+91"/>
        <source>Custom Fan Curve Dynamics &amp; Control Points</source>
        <translation>Dinámica y puntos de control de curva personalizada</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Multi-point linear temperature ramp curve mapped to cooling PWM controllers.</source>
        <translation>Curva lineal de temperatura multipunto asignada a los controladores PWM de refrigeración.</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Open Curve Studio &amp; Live Tuner ↗</source>
        <translation>Abrir estudio de curvas y sintonizador en vivo ↗</translation>
    </message>
    <message>
        <location line="+128"/>
        <source>Curve Presets:</source>
        <translation>Ajustes predefinidos de curva:</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Zero-dB Stealth</source>
        <translation>Zero-dB Silencioso</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Aggressive</source>
        <translation>Agresivo</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Stepped</source>
        <translation>Escalonado</translation>
    </message>
    <message>
        <location line="+64"/>
        <source>Response Smoothing</source>
        <translation>Suavizado de respuesta</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Active — GPU changes are rate-limited (%1°C hysteresis).</source>
        <translation>Activo — Los cambios de GPU están limitados por velocidad (%1°C histéresis).</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Disabled — GPU changes apply immediately.</source>
        <translation>Deshabilitado — Los cambios de GPU se aplican de inmediato.</translation>
    </message>
    <message>
        <location line="+70"/>
        <source>Disabled — does not alter fan profiles on battery.</source>
        <translation>Deshabilitado — No altera los perfiles de ventilador con batería.</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Active — controllable GPU fan uses Silent on battery.</source>
        <translation>Activo — El ventilador controlable de la GPU utiliza Silencioso con batería.</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Armed — uses Silent when battery power begins.</source>
        <translation>Preparado — Utiliza Silencioso cuando comienza la energía de la batería.</translation>
    </message>
    <message>
        <location line="-943"/>
        <source>Optimized profile dynamically balancing thermal dissipation and acoustic comfort.</source>
        <translation>Perfil optimizado que equilibra dinámicamente la disipación térmica y el confort acústico.</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Aggressive cooling profile providing maximum sustained airflow for heavy workloads.</source>
        <translation>Perfil de refrigeración agresivo que proporciona el máximo flujo de aire continuo para cargas pesadas.</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Fixed fan speed percentage defined directly by the user slider.</source>
        <translation>Porcentaje fijo de velocidad del ventilador definido directamente por el control deslizante.</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Default automatic profile managed natively by hardware VBIOS and kernel drivers.</source>
        <translation>Perfil automático predeterminado administrado de forma nativa por VBIOS y controladores del núcleo.</translation>
    </message>
    <message>
        <location line="+128"/>
        <source>Refresh fan telemetry</source>
        <translation>Actualizar telemetría de ventiladores</translation>
    </message>
    <message>
        <location line="-62"/>
        <source>Done ✓</source>
        <translation>Listo ✓</translation>
    </message>
    <message>
        <location line="+27"/>
        <source>Fan Setup Wizard</source>
        <translation>Asistente de configuración de ventiladores</translation>
    </message>
    <message>
        <location line="+128"/>
        <source>Fan Device</source>
        <translation>Dispositivo de ventilador</translation>
    </message>
    <message>
        <location line="+76"/>
        <source>RPM</source>
        <translation>RPM</translation>
    </message>
    <message>
        <location line="-21"/>
        <location line="+30"/>
        <location line="+33"/>
        <source>--</source>
        <translation>--</translation>
    </message>
    <message>
        <location line="-30"/>
        <source>0 RPM</source>
        <translation>0 RPM</translation>
    </message>
    <message>
        <location line="+189"/>
        <source>Optimization Profiles &amp; Control</source>
        <translation>Perfiles de Optimización y Control</translation>
    </message>
    <message>
        <location line="+20"/>
        <source>ACTIVE: %1</source>
        <translation>ACTIVO: %1</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>MANAGED: %1</source>
        <translation>ADMINISTRADO: %1</translation>
    </message>
    <message>
        <location line="+403"/>
        <source>Battery Profile Sync</source>
        <translation>Sincronización de perfil de batería</translation>
    </message>
</context>
<context>
    <name>FanRescanPopup</name>
    <message>
        <location filename="../src/qml/components/FanRescanPopup.qml" line="+17"/>
        <location line="+73"/>
        <source>Initializing Hardware Probe...</source>
        <translation>Inicializando sondeo de hardware...</translation>
    </message>
    <message>
        <location line="+117"/>
        <source>Fan Setup &amp; Discovery Wizard</source>
        <translation>Asistente de detección y configuración de ventiladores</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Scanning Thermal &amp; Fan Controllers...</source>
        <translation>Escaneando controladores térmicos y de ventilador...</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Hardware Discovery Complete</source>
        <translation>Detección de hardware completada</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Please wait while hardware sensors are probed...</source>
        <translation>Espere mientras se sondean los sensores de hardware...</translation>
    </message>
    <message>
        <location line="+35"/>
        <source>Close</source>
        <translation>Cerrar</translation>
    </message>
    <message>
        <location line="+40"/>
        <source>What will this hardware setup wizard do?</source>
        <translation>¿Qué hará este asistente de configuración de hardware?</translation>
    </message>
    <message>
        <location line="+25"/>
        <source>Kernel &amp; HWMON Thermal Zones</source>
        <translation>Zonas térmicas del kernel y HWMON</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Scans /sys/class/thermal and motherboard hardware monitoring chips</source>
        <translation>Escanea /sys/class/thermal y los chips de monitoreo de hardware de la placa base</translation>
    </message>
    <message>
        <location line="+26"/>
        <source>NVIDIA NV-CONTROL &amp; Fan Tachometers</source>
        <translation>NVIDIA NV-CONTROL y tacómetros de ventilador</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Probes dedicated GPU cooling channels and PWM control registers</source>
        <translation>Sondea los canales de refrigeración de la GPU y los registros de control PWM</translation>
    </message>
    <message>
        <location line="+26"/>
        <source>Profile &amp; Zero-dB Calibration</source>
        <translation>Calibración de perfil y Zero-dB</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Recalibrates zero-RPM stops, baseline curves, and safety overrides</source>
        <translation>Recalibra las paradas a cero RPM, las curvas base y las anulaciones de seguridad</translation>
    </message>
    <message>
        <location line="+28"/>
        <source>Non-intrusive probe: Fan speeds will not be interrupted during this scan.</source>
        <translation>Sonda no intrusiva: las velocidades del ventilador no se interrumpirán durante este escaneo.</translation>
    </message>
    <message>
        <location line="-321"/>
        <source>Preparing a read-only hardware probe...</source>
        <translation>Preparando una sonda de hardware de solo lectura...</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Probing Linux HWMON, ACPI, and NVIDIA fan interfaces...</source>
        <translation>Sondeando las interfaces de ventiladores Linux HWMON, ACPI y NVIDIA...</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Scan complete: %1 readable fan channel(s) found.</source>
        <translation>Escaneo completado: %1 canal(es) de ventilador legible(s) encontrado(s).</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Scan complete: no readable fan channels were exposed.</source>
        <translation>Escaneo completado: no se expusieron canales de ventilador legibles.</translation>
    </message>
    <message>
        <location line="+102"/>
        <source>Enumerate cooling fans and refresh exposed sensor telemetry</source>
        <translation>Enumerar los ventiladores de refrigeración y actualizar la telemetría de sensores expuesta</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Detected cooling telemetry is ready to review</source>
        <translation>La telemetría de refrigeración detectada está lista para su revisión</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>No readable fan telemetry was exposed by this system</source>
        <translation>Este sistema no expuso telemetría de ventiladores legible</translation>
    </message>
    <message>
        <location line="+404"/>
        <source>Telemetry Refresh &amp; Sensor Sync</source>
        <translation>Actualización de telemetría y sincronización de sensores</translation>
    </message>
    <message>
        <location line="+42"/>
        <source>Hardware Scan Completed</source>
        <translation>Escaneo de hardware completado</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Scan Completed Without Readable Fan Channels</source>
        <translation>Escaneo completado sin canales de ventilador legibles</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>%1 cooling channel(s) reported live telemetry. %2</source>
        <translation>%1 canal(es) de refrigeración reportaron telemetría en vivo. %2</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>The scan is complete, but firmware or the driver did not expose readable fan telemetry.</source>
        <translation>El escaneo se completó, pero el firmware o el controlador no expusieron telemetría legible de ventiladores.</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Discovered Cooling Channels</source>
        <translation>Canales de refrigeración detectados</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>found</source>
        <translation>encontrados</translation>
    </message>
    <message>
        <location line="+57"/>
        <source>Fan Channel</source>
        <translation>Canal de ventilador</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>%1 RPM • %2%</source>
        <translation>%1 RPM • %2%</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>0 RPM • Zero-dB / Passive</source>
        <translation>0 RPM • Zero-dB / Pasivo</translation>
    </message>
    <message>
        <location line="+19"/>
        <source>Controllable</source>
        <translation>Controlable</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Monitored</source>
        <translation>Solo monitorización</translation>
    </message>
    <message>
        <location line="+27"/>
        <source>ACPI Standard Thermal Mode</source>
        <translation>Modo térmico estándar ACPI</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Direct PWM fan tachometers are not exposed by the hardware. Thermal management is handled automatically via motherboard ACPI profiles.</source>
        <translation>Los tacómetros PWM directos del ventilador no están expuestos por el hardware. La gestión térmica se maneja automáticamente mediante los perfiles ACPI de la placa base.</translation>
    </message>
    <message>
        <location line="+16"/>
        <source>Testing Airflow... (%1s)</source>
        <translation>Probando flujo de aire... (%1s)</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Quick Acoustic Test (4s)</source>
        <translation>Prueba acústica rápida (4s)</translation>
    </message>
    <message>
        <location line="+42"/>
        <source>A readable fan channel was found, but this hardware does not expose manual fan control.</source>
        <translation>Se encontró un canal de ventilador legible, pero este hardware no expone control manual del ventilador.</translation>
    </message>
    <message>
        <location line="-443"/>
        <source>Cancel</source>
        <translation>Cancelar</translation>
    </message>
    <message>
        <location line="+28"/>
        <source>Run Setup Wizard</source>
        <translation>Ejecutar asistente</translation>
    </message>
    <message>
        <location line="+118"/>
        <source>Linux HWMON &amp; ACPI Controllers</source>
        <translation>Controladores Linux HWMON y ACPI</translation>
    </message>
    <message>
        <location line="+23"/>
        <source>NVIDIA GPU NV-CONTROL Interfaces</source>
        <translation>Interfaces NVIDIA GPU NV-CONTROL</translation>
    </message>
    <message>
        <location line="+283"/>
        <source>Done</source>
        <translation>Listo</translation>
    </message>
</context>
<context>
    <name>FanSettingsPopup</name>
    <message>
        <location filename="../src/qml/components/FanSettingsPopup.qml" line="+617"/>
        <source>Silent</source>
        <translation>Silencioso</translation>
    </message>
    <message>
        <location line="+1"/>
        <location line="+270"/>
        <source>Balanced</source>
        <translation>Equilibrado</translation>
    </message>
    <message>
        <location line="-269"/>
        <source>Performance</source>
        <translation>Rendimiento</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Manual</source>
        <translation>Manual</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Custom</source>
        <translation>Personalizado</translation>
    </message>
    <message>
        <location line="-5"/>
        <source>Auto</source>
        <translation>Automático</translation>
    </message>
    <message>
        <location line="-487"/>
        <source>Acoustic priority profile. Minimizes fan noise and delays speed ramp-up for quiet operation.</source>
        <translation>Perfil de prioridad acústica. Minimiza el ruido del ventilador para un funcionamiento silencioso.</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Dynamically balances thermal dissipation and acoustic comfort based on workload.</source>
        <translation>Equilibra dinámicamente la disipación térmica y el confort acústico según la carga de trabajo.</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Aggressive cooling profile providing maximum sustained airflow for heavy loads.</source>
        <translation>Perfil de refrigeración agresivo que proporciona el máximo flujo de aire continuo.</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Locked fixed fan speed percentage set directly by the manual slider.</source>
        <translation>Porcentaje de velocidad fija bloqueada establecida directamente mediante el control manual.</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Multi-point custom temperature-to-speed fan curve with smooth interpolation.</source>
        <translation>Curva de temperatura a velocidad personalizada multipunto con interpolación suave.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Automatic cooling curve managed natively by hardware VBIOS / BIOS thermal controllers.</source>
        <translation>Curva de refrigeración automática administrada de forma nativa por controladores térmicos de hardware.</translation>
    </message>
    <message>
        <location line="+385"/>
        <location line="+46"/>
        <source>--</source>
        <translation>--</translation>
    </message>
    <message>
        <location line="-44"/>
        <source>0 RPM</source>
        <translation>0 RPM</translation>
    </message>
    <message>
        <location line="+675"/>
        <source>Native automatic curve dynamically controlled by hardware thermals and firmware.</source>
        <translation>Curva automática nativa controlada dinámicamente por la temperatura del hardware y firmware.</translation>
    </message>
    <message>
        <location line="+172"/>
        <source>Reset to Auto</source>
        <translation>Restablecer a Auto</translation>
    </message>
    <message>
        <location line="-980"/>
        <source>Close</source>
        <translation>Cerrar</translation>
    </message>
    <message>
        <location line="-231"/>
        <source>The fan controller rejected this change.</source>
        <translation>El controlador del ventilador rechazó este cambio.</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>The automatic fan mode could not be restored.</source>
        <translation>No se pudo restaurar el modo de ventilador automático.</translation>
    </message>
    <message>
        <location line="+98"/>
        <source>Fan Settings &amp; Dynamics</source>
        <translation>Ajustes y dinámica de ventilador</translation>
    </message>
    <message>
        <location line="+61"/>
        <source>Save name</source>
        <translation>Guardar nombre</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Rename fan</source>
        <translation>Renombrar ventilador</translation>
    </message>
    <message>
        <location line="+23"/>
        <source>Hardware Channel: %1 • Interface: %2</source>
        <translation>Canal de hardware: %1 • Interfaz: %2</translation>
    </message>
    <message>
        <location line="+69"/>
        <source>This channel provides telemetry only. Its BIOS, firmware, or driver owns fan control, so ro-Control will not present simulated settings.</source>
        <translation>Este canal solo proporciona telemetría. La BIOS, el firmware o el controlador administran el ventilador, por lo que ro-Control no presentará ajustes simulados.</translation>
    </message>
    <message>
        <location line="+32"/>
        <source>LIVE SPEED</source>
        <translation>VELOCIDAD EN VIVO</translation>
    </message>
    <message>
        <location line="+58"/>
        <source>TACHOMETER</source>
        <translation>TACÓMETRO</translation>
    </message>
    <message>
        <location line="+20"/>
        <source>Silent Zero-RPM Active</source>
        <translation>Modo silencioso Zero-RPM activo</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Active Airflow Cooling</source>
        <translation>Refrigeración activa por flujo de aire</translation>
    </message>
    <message>
        <location line="+24"/>
        <source>TEMPERATURE</source>
        <translation>TEMPERATURA</translation>
    </message>
    <message>
        <location line="+23"/>
        <source>Thermal Load Elevated</source>
        <translation>Carga térmica elevada</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Moderate Thermals</source>
        <translation>Temperatura moderada</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Optimal Thermal State</source>
        <translation>Estado térmico óptimo</translation>
    </message>
    <message>
        <location line="+15"/>
        <source>Select Optimization Profile</source>
        <translation>Seleccionar perfil de optimización</translation>
    </message>
    <message>
        <location line="+80"/>
        <source>Manual Fixed Fan Speed Target</source>
        <translation>Velocidad fija manual de ventilador</translation>
    </message>
    <message>
        <location line="+71"/>
        <source>Quick Speed Presets:</source>
        <translation>Ajustes rápidos de velocidad:</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>0% (Stop)</source>
        <translation>0% (Detener)</translation>
    </message>
    <message>
        <location line="+40"/>
        <source>Interactive Custom Fan Curve &amp; Presets</source>
        <translation>Curva personalizada interactiva y ajustes predefinidos</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Reset curve</source>
        <translation>Restablecer curva</translation>
    </message>
    <message>
        <location line="+37"/>
        <source>Restore the default balanced fan curve</source>
        <translation>Restaurar la curva equilibrada predeterminada del ventilador</translation>
    </message>
    <message>
        <location line="+18"/>
        <source>Presets:</source>
        <translation>Ajustes predefinidos:</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Zero-dB Stealth</source>
        <translation>Zero-dB Silencioso</translation>
    </message>
    <message>
        <location line="+20"/>
        <source>Aggressive</source>
        <translation>Agresivo</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Stepped</source>
        <translation>Escalonado</translation>
    </message>
    <message>
        <location line="+211"/>
        <source>Point %1 (%2°C)</source>
        <translation>Punto %1 (%2°C)</translation>
    </message>
    <message>
        <location line="+69"/>
        <source>Profile Cooling Dynamics</source>
        <translation>Dinámica de refrigeración del perfil</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Acoustic priority: Fans remain in Zero-dB silent state under 45°C, ramping gently to 50% at 68°C and 100% at 85°C.</source>
        <translation>Prioridad acústica: Ventiladores en silencio Zero-dB bajo 45°C, subiendo suavemente a 50% a 68°C y 100% a 85°C.</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Aggressive cooling: 45% minimum speed floor, ramping rapidly to 80% at 65°C and 100% at 82°C for heavy compute/gaming.</source>
        <translation>Refrigeración agresiva: 45% de velocidad mínima, subiendo rápidamente a 80% a 65°C y 100% a 82°C para cargas pesadas.</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Optimized baseline: 30% speed floor, dynamically balancing acoustic comfort and thermal dissipation.</source>
        <translation>Base optimizada: 30% de velocidad mínima, equilibrando confort acústico y disipación térmica.</translation>
    </message>
    <message>
        <location line="+19"/>
        <source>Emergency 100% Thermal Guard Threshold</source>
        <translation>Umbral de protección térmica de emergencia al 100%</translation>
    </message>
    <message>
        <location line="+77"/>
        <source>Fan configuration applied and saved successfully!</source>
        <translation>¡Configuración de ventilador aplicada y guardada correctamente!</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>Testing (100%)...</source>
        <translation>Probando (100%)...</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Quick Test 100%</source>
        <translation>Prueba rápida 100%</translation>
    </message>
    <message>
        <location line="+29"/>
        <source>Direct fan control unavailable</source>
        <translation>El control directo del ventilador no está disponible</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>GPU fan test could not start. Enable NVIDIA Coolbits / fan control first.</source>
        <translation>No se pudo iniciar la prueba del ventilador de la GPU. Active primero NVIDIA Coolbits o el control del ventilador.</translation>
    </message>
    <message>
        <location line="+47"/>
        <source>Apply &amp; Save Settings</source>
        <translation>Aplicar y guardar configuración</translation>
    </message>
</context>
<context>
    <name>GpuMonitor</name>
    <message>
        <location filename="../src/backend/monitor/gpumonitor.cpp" line="+669"/>
        <source>No NVIDIA GPU detected in this session.</source>
        <translation>No se detectó ninguna GPU NVIDIA en esta sesión.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>NVIDIA driver is not exposing telemetry on this system.</source>
        <translation>El controlador NVIDIA no expone telemetría en este sistema.</translation>
    </message>
    <message>
        <location line="+64"/>
        <source>GPU telemetry is being read from Linux metrics.</source>
        <translation>La telemetría de la GPU se lee desde métricas de Linux.</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>GPU temperature is being read from system sensors.</source>
        <translation>La temperatura de la GPU se lee desde los sensores del sistema.</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>GPU telemetry output could not be parsed.</source>
        <translation>No se pudo analizar la salida de telemetría de la GPU.</translation>
    </message>
    <message>
        <location line="+90"/>
        <source>GPU telemetry output did not contain usable metrics.</source>
        <translation>La salida de telemetría de la GPU no contenía métricas utilizables.</translation>
    </message>
    <message>
        <location line="+94"/>
        <source>GPU telemetry is being read from nvidia-smi.</source>
        <translation>La telemetría de la GPU se lee desde nvidia-smi.</translation>
    </message>
    <message>
        <location line="+122"/>
        <source>The selected process is no longer using the active GPU.</source>
        <translation>El proceso seleccionado ya no está usando la GPU activa.</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>The process could not be terminated. Check ownership and permissions.</source>
        <translation>No se pudo terminar el proceso. Verifique la propiedad y los permisos.</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Termination signal sent to the selected GPU process.</source>
        <translation>Señal de terminación enviada al proceso de GPU seleccionado.</translation>
    </message>
</context>
<context>
    <name>GpuNames</name>
    <message>
        <location filename="../src/backend/system/systeminfoprovider.cpp" line="+162"/>
        <source>Integrated Graphics Controller</source>
        <translation>Controlador de gráficos integrado</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Integrated Display Controller</source>
        <translation>Controlador de pantalla integrado</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Integrated Graphics</source>
        <translation>Gráficos integrados</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Graphics Controller</source>
        <translation>Controlador de gráficos</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Display Controller</source>
        <translation>Controlador de pantalla</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Core Processor</source>
        <translation>Procesador Core</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Integrated</source>
        <translation>Integrado</translation>
    </message>
</context>
<context>
    <name>HealthGuard</name>
    <message>
        <location filename="../src/backend/system/healthguard.cpp" line="+153"/>
        <source>GPU Critical Temperature Alert</source>
        <translation>Alerta crítica de temperatura de GPU</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>GPU temperature reached %1°C (critical limit: %2°C). Cooling is throttled.</source>
        <translation>La temperatura de la GPU alcanzó %1°C (límite crítico: %2°C). Refrigeración al máximo.</translation>
    </message>
    <message>
        <location line="+16"/>
        <source>GPU High Temperature Warning</source>
        <translation>Advertencia de alta temperatura de GPU</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>GPU temperature is %1°C (warning limit: %2°C).</source>
        <translation>La temperatura de la GPU es %1°C (límite de advertencia: %2°C).</translation>
    </message>
    <message>
        <location line="+15"/>
        <source>CPU Critical Temperature Alert</source>
        <translation>Alerta crítica de temperatura de CPU</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>CPU temperature reached %1°C (critical limit: %2°C).</source>
        <translation>La temperatura de la CPU alcanzó %1°C (límite crítico: %2°C).</translation>
    </message>
    <message>
        <location line="+15"/>
        <source>CPU High Temperature Warning</source>
        <translation>Advertencia de alta temperatura de CPU</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>CPU temperature is %1°C (warning limit: %2°C).</source>
        <translation>La temperatura de la CPU es de %1°C (límite de advertencia: %2°C).</translation>
    </message>
</context>
<context>
    <name>LanguageManager</name>
    <message>
        <location filename="../src/backend/system/languagemanager.cpp" line="+28"/>
        <source>English</source>
        <translation>Inglés</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>German</source>
        <translation>Alemán</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Spanish</source>
        <translation>Español</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Turkish</source>
        <translation>Turco</translation>
    </message>
</context>
<context>
    <name>Main</name>
    <message>
        <location filename="../src/qml/Main.qml" line="+32"/>
        <source>ro-Control</source>
        <translation>ro-Control</translation>
    </message>
    <message>
        <location line="+20"/>
        <location line="+476"/>
        <source>Language</source>
        <translation>Idioma</translation>
    </message>
    <message>
        <location line="-471"/>
        <source>Dark</source>
        <translation>Oscuro</translation>
    </message>
    <message>
        <location line="+2"/>
        <location line="+1"/>
        <source>Light</source>
        <translation>Claro</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Unknown</source>
        <translation>Desconocido</translation>
    </message>
    <message>
        <location line="+8"/>
        <location line="+335"/>
        <source>Desktop</source>
        <translation>Escritorio</translation>
    </message>
    <message>
        <location line="-326"/>
        <location line="+330"/>
        <source>AC</source>
        <translation>Alimentación CA</translation>
    </message>
    <message>
        <location line="-328"/>
        <location line="+330"/>
        <source>Battery</source>
        <translation>Batería</translation>
    </message>
    <message>
        <location line="-108"/>
        <source>System</source>
        <translation>Sistema</translation>
    </message>
    <message>
        <location line="+32"/>
        <source>System Monitor</source>
        <translation>Monitor del sistema</translation>
    </message>
    <message>
        <location line="+56"/>
        <source>Wayland</source>
        <translation>Wayland</translation>
    </message>
    <message>
        <location line="+51"/>
        <location line="+70"/>
        <source>Settings</source>
        <translation>Ajustes</translation>
    </message>
    <message>
        <location line="+69"/>
        <source>Follow system language</source>
        <translation>Seguir el idioma del sistema</translation>
    </message>
    <message>
        <location line="+18"/>
        <source>Theme</source>
        <translation>Tema</translation>
    </message>
    <message>
        <location line="+54"/>
        <source>Show advanced information</source>
        <translation>Mostrar información avanzada</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Reset appearance settings</source>
        <translation>Restablecer la configuración de apariencia</translation>
    </message>
    <message>
        <location line="-390"/>
        <source>Cooling &amp; Fans</source>
        <translation>Refrigeración y Ventiladores</translation>
    </message>
    <message>
        <location line="-64"/>
        <source>Driver</source>
        <translation>Controlador</translation>
    </message>
    <message>
        <location line="+32"/>
        <source>Monitor</source>
        <translation>Monitor</translation>
    </message>
    <message>
        <location filename="../src/main.cpp" line="+1094"/>
        <source>Open ro-Control</source>
        <translation>Abrir ro-Control</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Fan Profile</source>
        <translation>Perfil de ventilador</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Auto</source>
        <translation>Automático</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Silent</source>
        <translation>Silencioso</translation>
    </message>
    <message>
        <location line="+2"/>
        <location line="+28"/>
        <source>Balanced</source>
        <translation>Equilibrado</translation>
    </message>
    <message>
        <location line="-26"/>
        <location line="+28"/>
        <source>Performance</source>
        <translation>Rendimiento</translation>
    </message>
    <message>
        <location line="-6"/>
        <source>Power Preset</source>
        <translation>Ajuste de energía</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Eco</source>
        <translation>Eco</translation>
    </message>
    <message>
        <location line="+35"/>
        <source>Fan Profile: %1</source>
        <translation>Perfil de ventilador: %1</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Power Preset: %1</source>
        <translation>Preajuste de energía: %1</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Quit</source>
        <translation>Salir</translation>
    </message>
</context>
<context>
    <name>MonitorPage</name>
    <message>
        <location filename="../src/qml/pages/MonitorPage.qml" line="+105"/>
        <location line="+4"/>
        <location line="+4"/>
        <location line="+5"/>
        <location line="+3"/>
        <location line="+179"/>
        <location line="+18"/>
        <location line="+155"/>
        <location line="+20"/>
        <location line="+53"/>
        <location line="+18"/>
        <location line="+179"/>
        <location line="+8"/>
        <source>Unavailable</source>
        <translation>No disponible</translation>
    </message>
    <message>
        <location line="-647"/>
        <source>VM sensor unavailable</source>
        <translation>Sensor VM no disponible</translation>
    </message>
    <message>
        <location line="+28"/>
        <source>No NVIDIA GPU detected. CPU and memory monitoring remain available; NVIDIA telemetry, power controls, and process monitoring are disabled.</source>
        <translation>No se detectó ninguna GPU NVIDIA. La monitorización de CPU y memoria sigue disponible; la telemetría, el control de energía y la monitorización de procesos NVIDIA están deshabilitados.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>NVIDIA GPU telemetry is unavailable. Check the driver and session permissions, then refresh.</source>
        <translation>Telemetría de GPU NVIDIA no disponible. Verifique los permisos del controlador y la sesión, luego actualice.</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Silent (Acoustic)</source>
        <translation>Silencioso (Acústico)</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Balanced (Optimized)</source>
        <translation>Equilibrado (Optimizado)</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Performance (High Cooling)</source>
        <translation>Rendimiento (Alta Refrigeración)</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Manual (Fixed Speed)</source>
        <translation>Manual (Velocidad Fija)</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Custom Curve</source>
        <translation>Curva Personalizada</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Auto (VBIOS / Driver)</source>
        <translation>Automático (VBIOS / Controlador)</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>SAFETY OVERRIDE 100%</source>
        <translation>PROTECCIÓN DE SEGURIDAD 100%</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>HARDWARE AUTO</source>
        <translation>HARDWARE AUTOMÁTICO</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>HARDWARE MANAGED</source>
        <translation>ADMINISTRADO POR HARDWARE</translation>
    </message>
    <message>
        <location line="+104"/>
        <source>Refreshing telemetry…</source>
        <translation>Actualizando telemetría…</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Refresh telemetry</source>
        <translation>Actualizar telemetría</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>CPU</source>
        <translation>CPU</translation>
    </message>
    <message>
        <location line="+13"/>
        <location line="+246"/>
        <location line="+71"/>
        <source>USAGE</source>
        <translation>USO</translation>
    </message>
    <message>
        <location line="-299"/>
        <location line="+173"/>
        <source>TEMPERATURE</source>
        <translation>TEMPERATURA</translation>
    </message>
    <message>
        <location line="-97"/>
        <source>Active: %1
Click to switch GPU (%2 available)</source>
        <translation>Activo: %1
Haga clic para cambiar de GPU (%2 disponible)</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Active Graphics Processor</source>
        <translation>Procesador gráfico activo</translation>
    </message>
    <message>
        <location line="+77"/>
        <source>LOAD</source>
        <translation>CARGA</translation>
    </message>
    <message>
        <location line="+91"/>
        <location line="+76"/>
        <source>ALLOCATED</source>
        <translation>ASIGNADO</translation>
    </message>
    <message>
        <location line="-36"/>
        <source>ZRAM</source>
        <translation>ZRAM</translation>
    </message>
    <message>
        <location line="+57"/>
        <source>RAM: %1 MiB</source>
        <translation>RAM: %1 MiB</translation>
    </message>
    <message>
        <location line="+73"/>
        <source>Core / Memory Clocks</source>
        <translation>Frecuencias de núcleo / memoria</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Power Draw / TDP Limit</source>
        <translation>Consumo de energía / Límite TDP</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Thermals &amp; Hotspot</source>
        <translation>Temperaturas y Hotspot</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Core: %1°C</source>
        <translation>Núcleo: %1°C</translation>
    </message>
    <message>
        <location line="+1"/>
        <source> • Hotspot: %1°C</source>
        <translation> • Hotspot: %1°C</translation>
    </message>
    <message>
        <location line="+1"/>
        <source> • VRAM: %1°C</source>
        <translation> • VRAM: %1°C</translation>
    </message>
    <message>
        <location line="+95"/>
        <source>Draw:</source>
        <translation>Consumo:</translation>
    </message>
    <message>
        <location line="+31"/>
        <source>Limit:</source>
        <translation>Límite:</translation>
    </message>
    <message>
        <location line="+118"/>
        <source>Show less</source>
        <translation>Mostrar menos</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>All processes</source>
        <translation>Todos los procesos</translation>
    </message>
    <message>
        <location line="+120"/>
        <source>VRAM ALLOCATION</source>
        <translation>ASIGNACIÓN VRAM</translation>
    </message>
    <message>
        <location line="+135"/>
        <source>End Task</source>
        <translation>Finalizar tarea</translation>
    </message>
    <message>
        <location line="+62"/>
        <source>End GPU process?</source>
        <translation>¿Finalizar el proceso de la GPU?</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>%1 (PID %2) will be terminated. Unsaved work may be lost.</source>
        <translation>Se finalizará %1 (PID %2). El trabajo no guardado podría perderse.</translation>
    </message>
    <message>
        <location line="+18"/>
        <source>Cancel</source>
        <translation>Cancelar</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>End process</source>
        <translation>Finalizar proceso</translation>
    </message>
    <message>
        <location line="-984"/>
        <source>GPU</source>
        <translation>GPU</translation>
    </message>
    <message>
        <location line="+171"/>
        <source>Memory</source>
        <translation>Memoria</translation>
    </message>
    <message>
        <location line="+185"/>
        <source>GPU Performance &amp; Power</source>
        <translation>Rendimiento y energía de la GPU</translation>
    </message>
    <message>
        <location line="+25"/>
        <source>Dynamic Power</source>
        <translation>Energía dinámica</translation>
    </message>
    <message>
        <location line="-6"/>
        <source>Dynamic Clock</source>
        <translation>Reloj dinámico</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>VRAM Allocation</source>
        <translation>Asignación de VRAM</translation>
    </message>
    <message>
        <location line="+228"/>
        <source>GPU Task Manager &amp; Active Processes</source>
        <translation>Administrador de tareas y procesos activos de GPU</translation>
    </message>
    <message>
        <location line="+89"/>
        <source>No Active GPU Processes</source>
        <translation>No hay procesos activos en la GPU</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>No applications are currently allocating VRAM or compute resources on this GPU.</source>
        <translation>Ninguna aplicación está utilizando VRAM o recursos de cálculo en esta GPU actualmente.</translation>
    </message>
    <message>
        <location line="+24"/>
        <source>PID</source>
        <translation>PID</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>PROCESS NAME</source>
        <translation>NOMBRE DEL PROCESO</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>TYPE</source>
        <translation>TIPO</translation>
    </message>
    <message>
        <location line="+18"/>
        <source>ACTION</source>
        <translation>ACCIÓN</translation>
    </message>
    <message>
        <location line="+76"/>
        <source>Compute</source>
        <translation>Cálculo</translation>
    </message>
    <message>
        <location line="-382"/>
        <source>GPU Power &amp; Performance Management</source>
        <translation>Gestión de energía y rendimiento de GPU</translation>
    </message>
    <message>
        <location line="+83"/>
        <source>Eco</source>
        <translation>Eco</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Balanced</source>
        <translation>Equilibrado</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Performance</source>
        <translation>Rendimiento</translation>
    </message>
    <message>
        <location line="-84"/>
        <source>Power &amp; Performance Management</source>
        <translation>Gestión de energía y rendimiento</translation>
    </message>
    <message>
        <location line="-166"/>
        <source>Compression: %1×</source>
        <translation>Compresión: %1×</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>zswap enabled</source>
        <translation>zswap activado</translation>
    </message>
</context>
<context>
    <name>NvidiaDetector</name>
    <message>
        <location filename="../src/backend/nvidia/detector.cpp" line="+129"/>
        <source>NVIDIA Open Kernel Modules</source>
        <translation>Módulos abiertos del kernel NVIDIA</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>NVIDIA Driver</source>
        <translation>Controlador NVIDIA</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Installed, Restart Required</source>
        <translation>Instalado, reinicio requerido</translation>
    </message>
    <message>
        <location line="+20"/>
        <source>NVIDIA Proprietary Kernel Module detected</source>
        <translation>Módulo privativo del kernel NVIDIA detectado</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>NVIDIA Open Kernel Modules detected</source>
        <translation>Módulos abiertos del kernel NVIDIA detectados</translation>
    </message>
    <message>
        <location line="+16"/>
        <source>GPU: %1
Driver Version: %2
Secure Boot: %3
Session: %4
Active Stack: %5</source>
        <translation>GPU: %1
Versión del controlador: %2
Secure Boot: %3
Sesión: %4
Pila activa: %5</translation>
    </message>
    <message>
        <location line="-38"/>
        <source>Not Installed</source>
        <translation>No instalado</translation>
    </message>
    <message>
        <location line="+25"/>
        <source>Mixed driver state detected</source>
        <translation>Estado mixto de controlador detectado</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>No driver source detected</source>
        <translation>No se detectó origen de controlador</translation>
    </message>
    <message>
        <location line="+6"/>
        <location line="+3"/>
        <source>Unavailable</source>
        <translation>No disponible</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Enabled</source>
        <translation>Activado</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Disabled</source>
        <translation>Desactivado</translation>
    </message>
    <message>
        <location line="+1"/>
        <location line="+1"/>
        <source>Unknown</source>
        <translation>Desconocido</translation>
    </message>
</context>
<context>
    <name>NvidiaInstaller</name>
    <message>
        <location filename="../src/backend/nvidia/installer.cpp" line="+166"/>
        <source>No driver operation is running.</source>
        <translation>No hay ninguna operación de controlador en curso.</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Cancel requested. Waiting for the active command to stop safely...</source>
        <translation>Cancelación solicitada. Esperando a que el comando activo se detenga de forma segura...</translation>
    </message>
    <message>
        <location line="+18"/>
        <source>Closed-source NVIDIA driver license summary

This closed-source NVIDIA driver is provided under NVIDIA&apos;s driver software license. By accepting, you confirm that you have authority to accept the license terms and that ro-Control may start installing the closed-source driver packages.

Important points:
- The software is licensed, not sold.
- The license is limited, revocable, non-transferable, and non-sublicensable except where NVIDIA explicitly allows it.
- You may install and use copies of the driver software only as permitted by the license and applicable law.
- NVIDIA does not grant extra third-party patent, codec, standards, or content rights through this driver license.
- Open-source components, if any, remain governed by their own separate open-source licenses.
- If you do not accept the NVIDIA license terms, do not install or use the closed-source driver.

Choose Accept to continue with the closed-source installation, or Reject to cancel.</source>
        <translation>Resumen de la licencia del controlador NVIDIA de código cerrado

Este controlador NVIDIA de código cerrado se proporciona bajo la licencia de software de controladores de NVIDIA. Al aceptar, confirma que tiene autoridad para aceptar los términos de la licencia y que ro-Control puede iniciar la instalación de los paquetes del controlador de código cerrado.

Puntos importantes:
- El software se licencia, no se vende.
- La licencia es limitada, revocable, no transferible y no sublicenciable salvo que NVIDIA lo permita expresamente.
- Puede instalar y usar copias del software del controlador solo según lo permitido por la licencia y la legislación aplicable.
- NVIDIA no concede derechos adicionales de patentes, códecs, estándares o contenido de terceros mediante esta licencia del controlador.
- Los componentes de código abierto, si los hay, siguen sujetos a sus propias licencias de código abierto.
- Si no acepta los términos de la licencia de NVIDIA, no instale ni use el controlador de código cerrado.

Elija Aceptar para continuar con la instalación de código cerrado o Rechazar para cancelar.</translation>
    </message>
    <message>
        <location line="+41"/>
        <source>NVIDIA license review confirmation is required before installation.</source>
        <translation>Se requiere confirmar la revisión de la licencia NVIDIA antes de la instalación.</translation>
    </message>
    <message>
        <location line="+24"/>
        <source>Checking RPM Fusion repositories...</source>
        <translation>Comprobando repositorios RPM Fusion...</translation>
    </message>
    <message>
        <location line="+14"/>
        <location line="+159"/>
        <source>Platform version could not be detected.</source>
        <translation>No se pudo detectar la versión de la plataforma.</translation>
    </message>
    <message>
        <location line="-142"/>
        <location line="+158"/>
        <source>The active display session could not be detected as Wayland. ro-Control supports Wayland driver setup only.</source>
        <translation>No se pudo detectar la sesión de pantalla activa como Wayland. ro-Control solo admite la configuración del controlador en Wayland.</translation>
    </message>
    <message>
        <location line="-142"/>
        <source>Closed-source install packages for %1: %2</source>
        <translation>Paquetes de instalación de código cerrado para %1: %2</translation>
    </message>
    <message>
        <location line="+52"/>
        <location line="+146"/>
        <location line="+61"/>
        <location line="+47"/>
        <location line="+20"/>
        <source>Operation canceled by user.</source>
        <translation>Operación cancelada por el usuario.</translation>
    </message>
    <message>
        <location line="-273"/>
        <source>Installation failed: </source>
        <translation>La instalación falló: </translation>
    </message>
    <message>
        <location line="+18"/>
        <source>The closed-source NVIDIA driver was installed successfully. Please restart the system.</source>
        <translation>El controlador NVIDIA de código cerrado se instaló correctamente. Reinicie el sistema.</translation>
    </message>
    <message>
        <location line="+304"/>
        <source>Rebuilding NVIDIA kernel modules and initramfs...</source>
        <translation>Reconstruyendo módulos de kernel de NVIDIA e initramfs...</translation>
    </message>
    <message>
        <location line="+15"/>
        <source>Kernel module rebuild canceled by user.</source>
        <translation>Reconstrucción del módulo del kernel cancelada por el usuario.</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Kernel module rebuild failed: </source>
        <translation>Error al reconstruir el módulo del kernel: </translation>
    </message>
    <message>
        <location line="+17"/>
        <source>Kernel modules and initramfs rebuilt successfully.</source>
        <translation>Módulos de kernel e initramfs reconstruidos con éxito.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>NVIDIA kernel modules rebuilt successfully.</source>
        <translation>Módulos de kernel de NVIDIA reconstruidos con éxito.</translation>
    </message>
    <message>
        <location line="-210"/>
        <source>unknown error</source>
        <translation>error desconocido</translation>
    </message>
    <message>
        <location line="-429"/>
        <source>NVIDIA Open Kernel Modules detected. Run Deep Clean before installing the proprietary NVIDIA kernel module.</source>
        <translation>Módulos abiertos del kernel NVIDIA detectados. Ejecute Limpieza profunda antes de instalar el módulo privativo del kernel NVIDIA.</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Proprietary NVIDIA kernel module detected. Run Deep Clean before installing NVIDIA Open Kernel Modules.</source>
        <translation>Módulo privativo del kernel NVIDIA detectado. Ejecute Limpieza profunda antes de instalar los módulos abiertos del kernel NVIDIA.</translation>
    </message>
    <message>
        <location line="+335"/>
        <source>Installing NVIDIA Open Kernel Modules...</source>
        <translation>Instalando módulos abiertos del kernel NVIDIA...</translation>
    </message>
    <message>
        <location line="+41"/>
        <source>NVIDIA Open Kernel Modules packages: %1</source>
        <translation>Paquetes de módulos abiertos del kernel NVIDIA: %1</translation>
    </message>
    <message>
        <location line="+45"/>
        <source>NVIDIA Open Kernel Modules installation failed: </source>
        <translation>Falló la instalación de los módulos abiertos del kernel NVIDIA: </translation>
    </message>
    <message>
        <location line="+19"/>
        <source>NVIDIA Open Kernel Modules were prepared successfully. Please restart the system.</source>
        <translation>Los módulos abiertos del kernel NVIDIA se prepararon correctamente. Por favor, reinicie el sistema.</translation>
    </message>
    <message>
        <location line="+29"/>
        <source>Removing the NVIDIA driver...</source>
        <translation>Eliminando el controlador NVIDIA...</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Driver removed successfully.</source>
        <translation>Controlador eliminado correctamente.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Removal failed: </source>
        <translation>La eliminación falló: </translation>
    </message>
    <message>
        <location line="+33"/>
        <source>Cleaning legacy driver leftovers...</source>
        <translation>Limpiando restos de controladores antiguos...</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Deep clean failed: </source>
        <translation>La limpieza profunda falló: </translation>
    </message>
    <message>
        <location line="+20"/>
        <source>DNF cache cleanup failed: </source>
        <translation>La limpieza de caché de DNF falló: </translation>
    </message>
    <message>
        <location line="+18"/>
        <source>Deep clean completed.</source>
        <translation>Limpieza profunda completada.</translation>
    </message>
    <message>
        <location line="-498"/>
        <source>Another driver operation is already running.</source>
        <translation>Ya hay otra operación de controlador en curso.</translation>
    </message>
    <message>
        <location line="+149"/>
        <source>Installing the closed-source NVIDIA driver with one privileged authorization...</source>
        <translation>Instalando el controlador NVIDIA cerrado con una única autorización privilegiada...</translation>
    </message>
    <message>
        <location line="+43"/>
        <location line="+146"/>
        <source>Detected %1 session via %2.</source>
        <translation>Sesión %1 detectada mediante %2.</translation>
    </message>
    <message>
        <location line="-184"/>
        <location line="+40"/>
        <location line="+146"/>
        <source>Wayland</source>
        <translation>Wayland</translation>
    </message>
    <message>
        <location line="-427"/>
        <source>No NVIDIA GPU or installed NVIDIA driver was detected. In a virtual machine, attach or passthrough an NVIDIA GPU before starting driver installation.</source>
        <translation>No se detectó una GPU NVIDIA ni un controlador NVIDIA instalado. En una máquina virtual, conecte o haga passthrough de una GPU NVIDIA antes de iniciar la instalación del controlador.</translation>
    </message>
    <message>
        <location line="+284"/>
        <location line="+146"/>
        <source>session probe</source>
        <translation>sondeo de sesión</translation>
    </message>
    <message>
        <location line="+157"/>
        <source>Legacy NVIDIA cleanup completed.</source>
        <translation>Limpieza de NVIDIA antiguo completada.</translation>
    </message>
</context>
<context>
    <name>NvidiaUpdater</name>
    <message>
        <location filename="../src/backend/nvidia/updater.cpp" line="+665"/>
        <source>Update failed: </source>
        <translation>La actualización falló: </translation>
    </message>
    <message>
        <location line="+53"/>
        <source>Driver updated successfully. Please restart the system.</source>
        <translation>Controlador actualizado correctamente. Reinicie el sistema.</translation>
    </message>
    <message>
        <location line="-440"/>
        <location line="+18"/>
        <location line="+279"/>
        <source>dnf not found.</source>
        <translation>dnf no encontrado.</translation>
    </message>
    <message>
        <location line="-513"/>
        <source>No NVIDIA GPU or installed NVIDIA driver was detected. In a virtual machine, attach or passthrough an NVIDIA GPU before starting driver updates.</source>
        <translation>No se detectó una GPU NVIDIA ni un controlador NVIDIA instalado. En una máquina virtual, conecte o haga passthrough de una GPU NVIDIA antes de iniciar actualizaciones del controlador.</translation>
    </message>
    <message>
        <location line="+207"/>
        <source>Official NVIDIA driver sources are reachable. You can install the driver now.</source>
        <translation>Las fuentes oficiales de controladores NVIDIA son accesibles. Puede instalar el controlador ahora.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Latest official NVIDIA driver version: %1</source>
        <translation>Última versión oficial del controlador NVIDIA: %1</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>No official NVIDIA driver version could be retrieved.</source>
        <translation>No se pudo obtener ninguna versión oficial del controlador NVIDIA.</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Official NVIDIA update found: %1</source>
        <translation>Actualización oficial de NVIDIA encontrada: %1</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Driver matches the latest official NVIDIA production branch.</source>
        <translation>El controlador coincide con la rama oficial de producción más reciente de NVIDIA.</translation>
    </message>
    <message>
        <location line="+25"/>
        <source>Update found (version details unavailable).</source>
        <translation>Actualización encontrada (detalles de versión no disponibles).</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Update found: %1</source>
        <translation>Actualización encontrada: %1</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Driver is up to date. No new version found.</source>
        <translation>El controlador está actualizado. No se encontró una versión nueva.</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Update check failed: %1</source>
        <translation>Falló la comprobación de actualizaciones: %1</translation>
    </message>
    <message>
        <location line="+26"/>
        <source>Another driver operation is already running.</source>
        <translation>Ya hay otra operación de controlador en curso.</translation>
    </message>
    <message>
        <location line="+17"/>
        <source>No driver operation is running.</source>
        <translation>No hay ninguna operación de controlador en curso.</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Cancel requested. Waiting for the active command to stop safely...</source>
        <translation>Cancelación solicitada. Esperando a que el comando activo se detenga de forma segura...</translation>
    </message>
    <message>
        <location line="+224"/>
        <source>The active display session could not be detected as Wayland. ro-Control supports Wayland driver setup only.</source>
        <translation>No se pudo detectar la sesión de pantalla activa como Wayland. ro-Control solo admite la configuración del controlador en Wayland.</translation>
    </message>
    <message>
        <location line="+72"/>
        <source>unknown error</source>
        <translation>error desconocido</translation>
    </message>
    <message>
        <location line="-15"/>
        <source>Detected %1 session via %2.</source>
        <translation>Sesión %1 detectada mediante %2.</translation>
    </message>
    <message>
        <location line="-18"/>
        <location line="+20"/>
        <source>Wayland</source>
        <translation>Wayland</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>session probe</source>
        <translation>sondeo de sesión</translation>
    </message>
    <message>
        <location line="-157"/>
        <source>Starting update check...</source>
        <translation>Iniciando comprobación de actualizaciones...</translation>
    </message>
    <message>
        <location line="+111"/>
        <source>Selected version not found in the repository.</source>
        <translation>La versión seleccionada no se encontró en el repositorio.</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Updating NVIDIA driver to the latest version...</source>
        <translation>Actualizando el controlador NVIDIA a la versión más reciente...</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Switching NVIDIA driver to selected version: %1</source>
        <translation>Cambiando el controlador NVIDIA a la versión seleccionada: %1</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Driver transaction kernel package: `%1`</source>
        <translation>Paquete de kernel de la transacción del controlador: `%1`</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Driver transaction packages for %1: %2</source>
        <translation>Paquetes de transacción del controlador para %1: %2</translation>
    </message>
    <message>
        <location line="+32"/>
        <source>Operation canceled by user.</source>
        <translation>Operación cancelada por el usuario.</translation>
    </message>
    <message>
        <location line="+18"/>
        <source>Driver is already at the latest available version.</source>
        <translation>El controlador ya está en la versión más reciente disponible.</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Selected driver version is already installed.</source>
        <translation>La versión seleccionada del controlador ya está instalada.</translation>
    </message>
    <message>
        <location line="+32"/>
        <source>Latest version installed successfully. Please restart the system.</source>
        <translation>La versión más reciente se instaló correctamente. Reinicie el sistema.</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Selected version applied successfully. Please restart the system.</source>
        <translation>La versión seleccionada se aplicó correctamente. Reinicie el sistema.</translation>
    </message>
</context>
<context>
    <name>ProcessMonitor</name>
    <message>
        <location filename="../src/backend/monitor/processmonitor.cpp" line="+191"/>
        <source>Process telemetry is being read from /proc.</source>
        <translation>La telemetría de procesos se está leyendo desde /proc.</translation>
    </message>
    <message>
        <location line="+15"/>
        <location line="+23"/>
        <source>Invalid process id.</source>
        <translation>ID de proceso no válido.</translation>
    </message>
    <message>
        <location line="-9"/>
        <source>Failed to send signal %1 to process %2.</source>
        <translation>No se pudo enviar la señal %1 al proceso %2.</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Nice value must be between -20 and 19.</source>
        <translation>El valor nice debe estar entre -20 y 19.</translation>
    </message>
    <message>
        <location line="+15"/>
        <source>Failed to renice process %1 to %2.</source>
        <translation>No se pudo cambiar el nice del proceso %1 a %2.</translation>
    </message>
</context>
<context>
    <name>RefreshToolButton</name>
    <message>
        <location filename="../src/qml/components/RefreshToolButton.qml" line="+13"/>
        <source>Refresh</source>
        <translation>Actualizar</translation>
    </message>
</context>
<context>
    <name>SidebarMenu</name>
    <message>
        <location filename="../src/qml/components/SidebarMenu.qml" line="+20"/>
        <source>Install</source>
        <translation>Instalar</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Expert</source>
        <translation>Experto</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Monitor</source>
        <translation>Monitor</translation>
    </message>
</context>
<context>
    <name>SystemMonitorPage</name>
    <message>
        <location filename="../src/qml/pages/SystemMonitorPage.qml" line="+82"/>
        <location line="+156"/>
        <location line="+50"/>
        <source>CPU %</source>
        <translation>CPU %</translation>
    </message>
    <message>
        <location line="-205"/>
        <location line="+160"/>
        <source>Memory</source>
        <translation>Memoria</translation>
    </message>
    <message>
        <location line="-159"/>
        <location line="+164"/>
        <location line="+38"/>
        <source>PID</source>
        <translation>PID</translation>
    </message>
    <message>
        <location line="-196"/>
        <source>process</source>
        <translation>proceso</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>SIGTERM sent to PID %1 (%2).</source>
        <translation>SIGTERM enviado al PID %1 (%2).</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Failed to terminate PID %1. See status message.</source>
        <translation>No se pudo terminar el PID %1. Consulte el mensaje de estado.</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>PID %1 reniced to %2.</source>
        <translation>PID %1 ajustado a nice %2.</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Failed to renice PID %1. See status message.</source>
        <translation>No se pudo cambiar el nice del PID %1. Consulte el mensaje de estado.</translation>
    </message>
    <message>
        <location line="+18"/>
        <source>System Resources</source>
        <translation>Recursos del sistema</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Live overview of CPU, memory, disk, and network</source>
        <translation>Vista general en tiempo real de CPU, memoria, disco y red</translation>
    </message>
    <message>
        <location line="+11"/>
        <location line="+72"/>
        <source>Processes</source>
        <translation>Procesos</translation>
    </message>
    <message>
        <location line="-67"/>
        <source>running</source>
        <translation>en ejecución</translation>
    </message>
    <message>
        <location line="+0"/>
        <location line="+13"/>
        <location line="+13"/>
        <source>unavailable</source>
        <translation>no disponible</translation>
    </message>
    <message>
        <location line="-19"/>
        <location line="+220"/>
        <source>Disks</source>
        <translation>Discos</translation>
    </message>
    <message>
        <location line="-215"/>
        <source>mounted block devices</source>
        <translation>dispositivos de bloque montados</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Network</source>
        <translation>Red</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>active interfaces</source>
        <translation>interfaces activas</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Throughput</source>
        <translation>Rendimiento</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>combined rx+tx</source>
        <translation>combinados rx+tx</translation>
    </message>
    <message>
        <location line="+31"/>
        <source>Top processes by CPU usage, memory footprint or PID. Right-click to manage.</source>
        <translation>Procesos principales por uso de CPU, consumo de memoria o PID. Clic derecho para gestionar.</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Sort by:</source>
        <translation>Ordenar por:</translation>
    </message>
    <message>
        <location line="+27"/>
        <source>Showing %1 of %2 processes</source>
        <translation>Mostrando %1 de %2 procesos</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Refresh</source>
        <translation>Actualizar</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>NAME</source>
        <translation>NOMBRE</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>MEM</source>
        <translation>MEM</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>STATE</source>
        <translation>ESTADO</translation>
    </message>
    <message>
        <location line="+29"/>
        <source>unknown</source>
        <translation>desconocido</translation>
    </message>
    <message>
        <location line="+30"/>
        <source>Kill</source>
        <translation>Terminar</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>+</source>
        <translation>+</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Increase nice by 1</source>
        <translation>Aumentar nice en 1</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Process telemetry is not available. The /proc filesystem is not mounted or readable.</source>
        <translation>La telemetría de procesos no está disponible. El sistema de archivos /proc no está montado o no es legible.</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Block device I/O counters and filesystem usage</source>
        <translation>Contadores de E/S de dispositivos de bloque y uso del sistema de archivos</translation>
    </message>
    <message>
        <location line="+54"/>
        <source>No block devices to display.</source>
        <translation>No hay dispositivos de bloque para mostrar.</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Network Interfaces</source>
        <translation>Interfaces de red</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Per-interface receive/transmit throughput and error counters</source>
        <translation>Rendimiento de recepción/transmisión por interfaz y contadores de errores</translation>
    </message>
    <message>
        <location line="+28"/>
        <source>rx</source>
        <translation>rx</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>tx</source>
        <translation>tx</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>err rx/tx</source>
        <translation>err rx/tx</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>drop</source>
        <translation>descartados</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>No network interfaces to display.</source>
        <translation>No hay interfaces de red para mostrar.</translation>
    </message>
    <message>
        <location line="+32"/>
        <source>Terminate process?</source>
        <translation>¿Terminar proceso?</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Send SIGTERM to PID %1 (%2)? Unsaved work in that process will be lost.</source>
        <translation>¿Enviar SIGTERM al PID %1 (%2)? El trabajo sin guardar de ese proceso se perderá.</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Cancel</source>
        <translation>Cancelar</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>End process</source>
        <translation>Finalizar proceso</translation>
    </message>
</context>
<context>
    <name>SystemPage</name>
    <message>
        <location filename="../src/qml/pages/SystemPage.qml" line="+73"/>
        <source>Bare Metal</source>
        <translation>Hardware físico</translation>
    </message>
    <message>
        <location line="-5"/>
        <location line="+7"/>
        <source>Secure Boot: On</source>
        <translation>Secure Boot: Activado</translation>
    </message>
    <message>
        <location line="-7"/>
        <location line="+7"/>
        <source>Secure Boot: Off</source>
        <translation>Secure Boot: Desactivado</translation>
    </message>
    <message>
        <location line="+195"/>
        <source>System health</source>
        <translation>Estado del sistema</translation>
    </message>
    <message>
        <location line="+45"/>
        <source>Hardware Specifications</source>
        <translation>Especificaciones de hardware</translation>
    </message>
    <message>
        <location line="-216"/>
        <source>Graphics Card (GPU)</source>
        <translation>Tarjeta gráfica (GPU)</translation>
    </message>
    <message>
        <location line="-35"/>
        <source>Driver: %1</source>
        <translation>Controlador: %1</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>GPU telemetry: Available</source>
        <translation>Telemetría de GPU: Disponible</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Live system information</source>
        <translation>Información del sistema en vivo</translation>
    </message>
    <message>
        <location line="+31"/>
        <location line="+50"/>
        <source>Processor (CPU)</source>
        <translation>Procesador (CPU)</translation>
    </message>
    <message>
        <location line="-49"/>
        <location line="+50"/>
        <source>Motherboard</source>
        <translation>Placa base</translation>
    </message>
    <message>
        <location line="-49"/>
        <location line="+50"/>
        <source>UEFI / BIOS</source>
        <translation>UEFI / BIOS</translation>
    </message>
    <message>
        <location line="-48"/>
        <location line="+49"/>
        <source>System Memory (RAM)</source>
        <translation>Memoria del sistema (RAM)</translation>
    </message>
    <message>
        <location line="-47"/>
        <location line="+57"/>
        <source>Video Memory (VRAM)</source>
        <translation>Memoria de vídeo (VRAM)</translation>
    </message>
    <message>
        <location line="-55"/>
        <source>Integrated Graphics Memory</source>
        <translation>Memoria de gráficos integrados</translation>
    </message>
    <message>
        <location line="+1"/>
        <location line="+55"/>
        <source>PCIe Link Interface</source>
        <translation>Interfaz de enlace PCIe</translation>
    </message>
    <message>
        <location line="+56"/>
        <location line="+1300"/>
        <source>Diagnostic report copied to clipboard.</source>
        <translation>Informe de diagnóstico copiado al portapapeles.</translation>
    </message>
    <message>
        <location line="-1295"/>
        <source>The report could not be copied. You can copy it manually from this preview.</source>
        <translation>No se pudo copiar el informe. Puede copiarlo manualmente desde esta vista previa.</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>System information refreshed.</source>
        <translation>Información del sistema actualizada.</translation>
    </message>
    <message>
        <location line="+34"/>
        <source>Refreshing…</source>
        <translation>Actualizando…</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Refresh</source>
        <translation>Actualizar</translation>
    </message>
    <message>
        <location line="+96"/>
        <source>No readable hardware details are currently exposed by this system.</source>
        <translation>Actualmente este sistema no expone detalles de hardware legibles.</translation>
    </message>
    <message>
        <location line="+76"/>
        <source>Software and platform details are temporarily unavailable.</source>
        <translation>Los detalles del software y la plataforma no están disponibles temporalmente.</translation>
    </message>
    <message>
        <location line="+31"/>
        <source>Generate a shareable system report or restart directly into firmware setup.</source>
        <translation>Genere un informe del sistema para compartir o reinicie directamente en la configuración del firmware.</translation>
    </message>
    <message>
        <location line="+53"/>
        <source>Diagnostic Report</source>
        <translation>Informe de diagnóstico</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Preview, format, and share live system details</source>
        <translation>Previsualice, dé formato y comparta detalles del sistema en vivo</translation>
    </message>
    <message>
        <location line="+23"/>
        <source>Open</source>
        <translation>Abrir</translation>
    </message>
    <message>
        <location line="+59"/>
        <source>UEFI / BIOS Firmware</source>
        <translation>Firmware UEFI / BIOS</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Restart directly into firmware setup</source>
        <translation>Reiniciar directamente en la configuración del firmware</translation>
    </message>
    <message>
        <location line="+23"/>
        <source>Restart</source>
        <translation>Reiniciar</translation>
    </message>
    <message>
        <location line="+99"/>
        <source>System Diagnostic Report</source>
        <translation>Informe de diagnóstico del sistema</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>System hardware, kernel, driver and security telemetry snapshot</source>
        <translation>Instantánea de telemetría de hardware, kernel, controlador y seguridad del sistema</translation>
    </message>
    <message>
        <location line="+73"/>
        <source>Overview Cards</source>
        <translation>Tarjetas de resumen</translation>
    </message>
    <message>
        <location line="+25"/>
        <source>Code / Export</source>
        <translation>Código / Exportar</translation>
    </message>
    <message>
        <location line="+43"/>
        <source>Filter properties...</source>
        <translation>Filtrar propiedades...</translation>
    </message>
    <message>
        <location line="+39"/>
        <source>Format:</source>
        <translation>Formato:</translation>
    </message>
    <message>
        <location line="+35"/>
        <location line="+35"/>
        <source>Plain Text</source>
        <translation>Texto plano</translation>
    </message>
    <message>
        <location line="+60"/>
        <source>Action:</source>
        <translation>Acción:</translation>
    </message>
    <message>
        <location line="+34"/>
        <location line="+35"/>
        <source>Copy on Open</source>
        <translation>Copiar al abrir</translation>
    </message>
    <message>
        <location line="-34"/>
        <location line="+33"/>
        <source>Preview</source>
        <translation>Vista previa</translation>
    </message>
    <message>
        <location line="+120"/>
        <source>items</source>
        <translation>elementos</translation>
    </message>
    <message>
        <location line="+100"/>
        <source>No matching properties found</source>
        <translation>No se encontraron propiedades coincidentes</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Try a different search term or clear the filter.</source>
        <translation>Pruebe con un término de búsqueda diferente o limpie el filtro.</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Clear Filter</source>
        <translation>Limpiar filtro</translation>
    </message>
    <message>
        <location line="+85"/>
        <source>Copied to clipboard!</source>
        <translation>¡Copiado al portapapeles!</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Close</source>
        <translation>Cerrar</translation>
    </message>
    <message>
        <location line="+29"/>
        <source>Copy Full Report</source>
        <translation>Copiar informe completo</translation>
    </message>
    <message>
        <location line="+31"/>
        <source>The report could not be copied. Select and copy the text manually.</source>
        <translation>No se pudo copiar el informe. Seleccione y copie el texto manualmente.</translation>
    </message>
    <message>
        <location line="+62"/>
        <source>Reboot to UEFI / BIOS</source>
        <translation>Reiniciar en UEFI / BIOS</translation>
    </message>
    <message>
        <location line="+24"/>
        <source>Make sure any unsaved work in other applications is saved before continuing.</source>
        <translation>Asegúrese de guardar el trabajo no guardado en otras aplicaciones antes de continuar.</translation>
    </message>
    <message>
        <location line="+34"/>
        <source>Cancel</source>
        <translation>Cancelar</translation>
    </message>
    <message>
        <location line="+30"/>
        <source>Restart Now ↻</source>
        <translation>Reiniciar ahora ↻</translation>
    </message>
    <message>
        <location line="+29"/>
        <source>Could not start a firmware reboot. Your system may not support this action or authorization was denied.</source>
        <translation>No se pudo iniciar el reinicio del firmware. Es posible que su sistema no admita esta acción o se denegó la autorización.</translation>
    </message>
    <message>
        <location line="-1593"/>
        <location line="+44"/>
        <source>Device &amp; Power</source>
        <translation>Dispositivo y energía</translation>
    </message>
    <message>
        <location line="+238"/>
        <source>Operating System &amp; Software Stack</source>
        <translation>Sistema operativo y software</translation>
    </message>
    <message>
        <location line="-272"/>
        <location line="+19"/>
        <source>Operating System</source>
        <translation>Sistema operativo</translation>
    </message>
    <message>
        <location line="-18"/>
        <location line="+20"/>
        <source>Desktop Environment</source>
        <translation>Entorno de escritorio</translation>
    </message>
    <message>
        <location line="-19"/>
        <location line="+18"/>
        <source>Linux Kernel</source>
        <translation>Kernel de Linux</translation>
    </message>
    <message>
        <location line="-16"/>
        <location line="+18"/>
        <source>Display Server / Session</source>
        <translation>Servidor de pantalla / Sesión</translation>
    </message>
    <message>
        <location line="-16"/>
        <location line="+36"/>
        <source>NVIDIA Driver</source>
        <translation>Controlador NVIDIA</translation>
    </message>
    <message>
        <location line="-35"/>
        <location line="+39"/>
        <source>Graphics &amp; Compute APIs</source>
        <translation>APIs de gráficos y cómputo</translation>
    </message>
    <message>
        <location line="-38"/>
        <source>Platform &amp; Security</source>
        <translation>Plataforma y seguridad</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Operating System &amp; Platform</source>
        <translation>Sistema operativo y plataforma</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Platform Security</source>
        <translation>Seguridad de la plataforma</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Processor &amp; Hardware</source>
        <translation>Procesador y hardware</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Graphics &amp; Accelerators</source>
        <translation>Gráficos y aceleradores</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Dedicated GPU</source>
        <translation>GPU dedicada</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Integrated GPU</source>
        <translation>GPU integrada</translation>
    </message>
    <message>
        <location line="+303"/>
        <source>Diagnostics &amp; System Controls</source>
        <translation>Diagnósticos y control del sistema</translation>
    </message>
    <message>
        <location line="+1134"/>
        <source>Your system will restart immediately and boot directly into the UEFI / BIOS firmware setup utility.</source>
        <translation>El sistema se reiniciará de inmediato y arrancará directamente en la utilidad de configuración UEFI / BIOS.</translation>
    </message>
</context>
<context>
    <name>T</name>
    <message>
        <location filename="../src/backend/asyncrunner.h" line="+49"/>
        <source>Starting privileged transaction batch (attempt %1). The exact commands and package manager output will appear below.</source>
        <translation>Iniciando lote de transacción privilegiada (intento %1). Los comandos exactos y la salida del gestor de paquetes aparecerán a continuación.</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Starting command (attempt %1): %2</source>
        <translation>Iniciando comando (intento %1): %2</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Command finished (attempt %1, exit %2, %3 ms): %4</source>
        <translation>Comando finalizado (intento %1, salida %2, %3 ms): %4</translation>
    </message>
</context>
<context>
    <name>UiPreferencesManager</name>
    <message>
        <location filename="../src/backend/system/uipreferencesmanager.cpp" line="+26"/>
        <source>Light</source>
        <translation>Claro</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>System</source>
        <translation>Sistema</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Dark</source>
        <translation>Oscuro</translation>
    </message>
</context>
</TS>
