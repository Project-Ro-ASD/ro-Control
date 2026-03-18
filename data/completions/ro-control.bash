_ro_control()
{
    local cur prev words cword
    _init_completion || return

    local commands="help version status diagnostics driver"
    local driver_commands="install remove update deep-clean"
    local global_opts="--help --version --diagnostics --json"
    local install_opts="--proprietary --open-source --accept-license"

    if [[ ${cword} -eq 1 ]]; then
        COMPREPLY=( $(compgen -W "${commands} ${global_opts}" -- "${cur}") )
        return
    fi

    case "${words[1]}" in
        status|diagnostics)
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
    esac

    COMPREPLY=( $(compgen -W "${global_opts}" -- "${cur}") )
}

complete -F _ro_control ro-control
