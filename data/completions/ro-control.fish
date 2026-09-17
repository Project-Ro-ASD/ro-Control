complete -c ro-control -f

complete -c ro-control -n "__fish_use_subcommand" -a help -d "Show usage information"
complete -c ro-control -n "__fish_use_subcommand" -a version -d "Show application version"
complete -c ro-control -n "__fish_use_subcommand" -a status -d "Show concise system and driver status"
complete -c ro-control -n "__fish_use_subcommand" -a diagnostics -d "Show full diagnostics snapshot"
complete -c ro-control -n "__fish_use_subcommand" -a driver -d "Manage NVIDIA drivers"
complete -c ro-control -n "__fish_use_subcommand" -a fan -d "Manage cooling and fan profiles"
complete -c ro-control -n "__fish_use_subcommand" -a power -d "Manage GPU power limits and profiles"
complete -c ro-control -n "__fish_use_subcommand" -a processes -d "List active GPU compute and display processes"
complete -c ro-control -n "__fish_use_subcommand" -a kill-process -d "Terminate a running GPU process"
complete -c ro-control -n "__fish_use_subcommand" -a gpus -d "List detected GPU adapters"
complete -c ro-control -n "__fish_use_subcommand" -a select-gpu -d "Select active GPU device by index"

complete -c ro-control -l help -d "Show usage information"
complete -c ro-control -s h -d "Show usage information"
complete -c ro-control -l version -d "Show application version"
complete -c ro-control -s v -d "Show application version"
complete -c ro-control -l diagnostics -s d -d "Legacy alias for diagnostics"
complete -c ro-control -l daemon -d "Run headless in background with D-Bus service"
complete -c ro-control -n "__fish_seen_subcommand_from status diagnostics processes gpus" -l json -d "Render output as JSON"

complete -c ro-control -n "__fish_seen_subcommand_from driver; and not __fish_seen_subcommand_from install remove update deep-clean" -a install -d "Install the NVIDIA driver"
complete -c ro-control -n "__fish_seen_subcommand_from driver; and not __fish_seen_subcommand_from install remove update deep-clean" -a remove -d "Remove installed NVIDIA packages"
complete -c ro-control -n "__fish_seen_subcommand_from driver; and not __fish_seen_subcommand_from install remove update deep-clean" -a update -d "Update the installed NVIDIA driver"
complete -c ro-control -n "__fish_seen_subcommand_from driver; and not __fish_seen_subcommand_from install remove update deep-clean" -a deep-clean -d "Remove legacy NVIDIA leftovers"

complete -c ro-control -n "__fish_seen_subcommand_from install" -l proprietary -d "Use the proprietary NVIDIA driver install path"
complete -c ro-control -n "__fish_seen_subcommand_from install" -l open-source -d "Use the community open-source graphics path"
complete -c ro-control -n "__fish_seen_subcommand_from install" -l accept-license -d "Confirm NVIDIA license review for the proprietary install path"

complete -c ro-control -n "__fish_seen_subcommand_from fan; and not __fish_seen_subcommand_from status set-speed set-mode set-smoothing reset" -a status -d "Show fan status"
complete -c ro-control -n "__fish_seen_subcommand_from fan; and not __fish_seen_subcommand_from status set-speed set-mode set-smoothing reset" -a set-speed -d "Set fan speed percentage"
complete -c ro-control -n "__fish_seen_subcommand_from fan; and not __fish_seen_subcommand_from status set-speed set-mode set-smoothing reset" -a set-mode -d "Set fan operating mode profile"
complete -c ro-control -n "__fish_seen_subcommand_from fan; and not __fish_seen_subcommand_from status set-speed set-mode set-smoothing reset" -a set-smoothing -d "Configure fan curve smoothing"
complete -c ro-control -n "__fish_seen_subcommand_from fan; and not __fish_seen_subcommand_from status set-speed set-mode set-smoothing reset" -a reset -d "Restore default automatic fan profile"

complete -c ro-control -n "__fish_seen_subcommand_from fan; and __fish_seen_subcommand_from status" -l json -d "Render fan status output as JSON"
complete -c ro-control -n "__fish_seen_subcommand_from fan; and __fish_seen_subcommand_from set-mode" -a "auto silent balanced performance manual custom" -d "Fan operating profile"

complete -c ro-control -n "__fish_seen_subcommand_from power; and not __fish_seen_subcommand_from status set-limit set-preset set-clocks set-persistence" -a status -d "Show GPU power status and limits"
complete -c ro-control -n "__fish_seen_subcommand_from power; and not __fish_seen_subcommand_from status set-limit set-preset set-clocks set-persistence" -a set-limit -d "Set GPU power limit in Watts"
complete -c ro-control -n "__fish_seen_subcommand_from power; and not __fish_seen_subcommand_from status set-limit set-preset set-clocks set-persistence" -a set-preset -d "Set power preset profile"
complete -c ro-control -n "__fish_seen_subcommand_from power; and not __fish_seen_subcommand_from status set-limit set-preset set-clocks set-persistence" -a set-clocks -d "Set GPU core and memory clock offsets"
complete -c ro-control -n "__fish_seen_subcommand_from power; and not __fish_seen_subcommand_from status set-limit set-preset set-clocks set-persistence" -a set-persistence -d "Enable or disable persistence mode"

complete -c ro-control -n "__fish_seen_subcommand_from power; and __fish_seen_subcommand_from status" -l json -d "Render power status output as JSON"
complete -c ro-control -n "__fish_seen_subcommand_from power; and __fish_seen_subcommand_from set-preset" -a "eco balanced performance custom" -d "Power preset profile"
complete -c ro-control -n "__fish_seen_subcommand_from power; and __fish_seen_subcommand_from set-persistence" -a "on off" -d "Persistence mode state"
