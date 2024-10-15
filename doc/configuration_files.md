# configuration files (.cfg)

The config files are based of .ini files and are simuler except:
- They do not have sections i.e [SECTION name]

## format

'#' repsents a comment

KEY_NAME = value

thats almost it

keys can not have spaces, values stop on new lines
unless quotation marks are used

keys and valyes need to be sporated by ether '=' ' ' or a combination

## Sytem configuration files

there currently is only one system config file `system.cfg` and it is as follows

|                      | |
|----------------------|-|
| `MAIN_INTERFACE_APP` | Tells the kernal what app to open on start up <br>Note that if the program exits in less then 0.25 seconds <br>The kernal will "abort" and stop loading the app other wise it loops