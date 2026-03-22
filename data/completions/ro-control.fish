complete -c ro-control -f

complete -c ro-control -n "__fish_use_subcommand" -a help -d "Show usage information"
complete -c ro-control -n "__fish_use_subcommand" -a version -d "Show application version"
complete -c ro-control -n "__fish_use_subcommand" -a status -d "Show concise system and driver status"
complete -c ro-control -n "__fish_use_subcommand" -a diagnostics -d "Show full diagnostics snapshot"
complete -c ro-control -n "__fish_use_subcommand" -a driver -d "Manage NVIDIA drivers"

complete -c ro-control -l help -d "Show usage information"
complete -c ro-control -s h -d "Show usage information"
complete -c ro-control -l version -d "Show application version"
complete -c ro-control -s v -d "Show application version"
complete -c ro-control -l diagnostics -s d -d "Legacy alias for diagnostics"
complete -c ro-control -n "__fish_seen_subcommand_from status diagnostics" -l json -d "Render output as JSON"

complete -c ro-control -n "__fish_seen_subcommand_from driver; and not __fish_seen_subcommand_from install remove update deep-clean" -a install -d "Install the NVIDIA driver"
complete -c ro-control -n "__fish_seen_subcommand_from driver; and not __fish_seen_subcommand_from install remove update deep-clean" -a remove -d "Remove installed NVIDIA packages"
complete -c ro-control -n "__fish_seen_subcommand_from driver; and not __fish_seen_subcommand_from install remove update deep-clean" -a update -d "Update the installed NVIDIA driver"
complete -c ro-control -n "__fish_seen_subcommand_from driver; and not __fish_seen_subcommand_from install remove update deep-clean" -a deep-clean -d "Remove legacy NVIDIA leftovers"

complete -c ro-control -n "__fish_seen_subcommand_from install" -l proprietary -d "Use the proprietary NVIDIA driver install path"
complete -c ro-control -n "__fish_seen_subcommand_from install" -l open-source -d "Use the NVIDIA open kernel module install path"
complete -c ro-control -n "__fish_seen_subcommand_from install" -l accept-license -d "Confirm NVIDIA license review for the proprietary install path"
