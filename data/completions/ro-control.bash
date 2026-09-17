_ro_control()
{
    local cur prev words cword
    _init_completion || return

    local commands="help version status diagnostics driver fan power processes kill-process gpus select-gpu"
    local driver_commands="install remove update deep-clean"
    local fan_commands="status set-speed set-mode set-smoothing reset"
    local power_commands="status set-limit set-preset set-clocks set-persistence"
    local power_presets="eco balanced performance custom"
    local fan_modes="auto silent balanced performance manual custom"
    local global_opts="--help --version --diagnostics --json --daemon"
    local install_opts="--proprietary --open-source --accept-license"

    if [[ ${cword} -eq 1 ]]; then
        COMPREPLY=( $(compgen -W "${commands} ${global_opts}" -- "${cur}") )
        return
    fi

    case "${words[1]}" in
        status|diagnostics|processes|gpus)
            COMPREPLY=( $(compgen -W "--json" -- "${cur}") )
            return
            ;;
        driver)
            if [[ ${cword} -eq 2 ]]; then
                COMPREPLY=( $(compgen -W "${driver_commands}" -- "${cur}") )
                return
            fi

            if [[ ${words[2]} == "install" ]]; then
                COMPREPLY=( $(compgen -W "${install_opts}" -- "${cur}") )
                return
            fi
            ;;
        fan)
            if [[ ${cword} -eq 2 ]]; then
                COMPREPLY=( $(compgen -W "${fan_commands}" -- "${cur}") )
                return
            fi

            if [[ ${words[2]} == "status" ]]; then
                COMPREPLY=( $(compgen -W "--json" -- "${cur}") )
                return
            fi

            if [[ ${words[2]} == "set-mode" ]]; then
                COMPREPLY=( $(compgen -W "${fan_modes}" -- "${cur}") )
                return
            fi
            ;;
        power)
            if [[ ${cword} -eq 2 ]]; then
                COMPREPLY=( $(compgen -W "${power_commands}" -- "${cur}") )
                return
            fi

            if [[ ${words[2]} == "status" ]]; then
                COMPREPLY=( $(compgen -W "--json" -- "${cur}") )
                return
            fi

            if [[ ${words[2]} == "set-preset" ]]; then
                COMPREPLY=( $(compgen -W "${power_presets}" -- "${cur}") )
                return
            fi

            if [[ ${words[2]} == "set-persistence" ]]; then
                COMPREPLY=( $(compgen -W "on off" -- "${cur}") )
                return
            fi
            ;;
    esac

    COMPREPLY=( $(compgen -W "${global_opts}" -- "${cur}") )
}

complete -F _ro_control ro-control
