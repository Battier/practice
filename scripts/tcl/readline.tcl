#!/bin/tclsh

set ::tcl_interactive 1

proc user_at_host { } {
    return $::tcl_platform(user)@[info hostname]
}

proc shorten_path {path} {
    set splitPath [file split $path]
    set result {}
    foreach fragment [lrange $splitPath 0 end-1] {
        lappend result [string index $fragment 0]
    }
    return [file join {*}$result [lindex $splitPath end]]
}

# Looks like this:
# tclsh-8.6.3 [Downloads] %
set path "tclsh-[info patchlevel] \[[pwd]\]% "

# Looks like the above but with color.
set pathColor "\033\[36mtclsh-[info patchlevel]\033\[0m \[\033\[34m[file tail [pwd]\033\[0m]\]\033\[31m % \033\[0m"

# A fish shell-style prompt with a clock.
# Looks like this:
# 14:49:52 user@host /h/d/Downloads>
set  fish "[clock format [clock seconds] -format {%H:%M:%S}] [user_at_host] [shorten_path [pwd]]> "

proc set_prompt {} {
    namespace eval tclreadline { 
         proc prompt1 {} { 
             return $::pathColor
         } 
    }
}

if {$tcl_interactive} {

    package require tclreadline

    # uncomment the following if block, if you
    # want `ls' executed after every `cd'. (This was
    # the default up to 0.8 == tclreadline_version.)
    #
    # if {"" == [info procs cd]} {
    #     catch {rename ::tclreadline::Cd ""}
    #     rename cd ::tclreadline::Cd
    #     proc cd {args} {
    #         if {[catch {eval ::tclreadline::Cd $args} message]} {
    #             puts stderr "$message"
    #         }
    #         tclreadline::ls
    #     }
    # }

    # uncomment the following line to use
    # tclreadline's fancy ls proc.
    #
    # namespace import tclreadline::ls

    # tclreadline::Print is on (`yes') by default.
    # This mimics the command echoing like in the
    # non-readline interactive tclsh.
    # If you don't like this, uncomment the following
    # line.
    #
    # tclreadline::Print no

    # uncomment the folling line, if you want
    # to change tclreadline's print behaviour
    # frequently with less typing.
    #
    # namespace import tclreadline::Print

    # store maximal this much lines in the history file
    #
    set ::tclreadline::historyLength 200

    set_prompt

    # go to tclrealdine's main loop.
    #
    ::tclreadline::Loop
}

