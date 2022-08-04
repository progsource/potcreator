# potcreator

Translations made "easier".

This tool can create `*.pot` files[^1] from different sources. So far 4 are
implemented: **GDScript**, **GDScene**, **JSON** and **SQLite**.

It is configurable with a simple JSON file.

Even that it started out to go together with Godot projects, it can also be used
outside of such a context.

**Info:** This repository is in maintenance mode. I do not work actively on it,
but if you create issues or Pull Requests, I will take a look. Time-wise that
might vary though.

## How to build

**Requirements:**

* [SCons](https://scons.org/)
* C++ 20
  * Windows: [mingw](https://www.mingw-w64.org/) (untested with others)
* Godot 4+ project for GDScript and GDScene modules

### Windows

Go in the terminal to this project directory and call `./build.bat`.

### Linux / OSX

Go in the terminal to this project directory and call `./build.sh`.

## How to configure

The JSON file has to be called `.potcreator.json` and the base looks like this:

```json
{
  "output" : "/template.pot",
  "displaySrc" : true,
  "modules" : [
  ]
}

```

`output` defines where the pot file shall later go to. The base path is always
the folder path to the configuration file.

`displaySrc` defines if it shall add comments into the pot file with where the
string comes from.

### GDScript

The module will walk through any given path and check `*.gd` files for `tr("")`.
Everything between the quotes will be counted as a translation key.

```json
{
  "output" : "/template.pot",
  "displaySrc" : true,
  "modules" : [
    {
      "name" : "gdscript",
      "data" : {
        "paths" : ["/scripts"]
      }
    }
  ]
}
```

Simply put into `paths` an array of paths where the tool shall look for
translation keys.

*This only works currently properly with text that has no line-breaks.*

### GDScene

This module will walk through any given path and check `*.tscn` files for
`text = ""` and `title = ""`. It will leave out texts that start with `~`.
Besides that everything between the quotes will be counted as a translation key.

```json
{
  "output" : "/template.pot",
  "displaySrc" : true,
  "modules" : [
    {
      "name" : "gdscene",
      "data" : {
        "paths" : ["/scenes"]
      }
    }
  ]
}
```

*This only works currently properly with text that has no line-breaks.*

### JSON

JSON files with strings in them also can contain translation keys. For example
if you had configs for multiple objects in a game. The configuration for these
would then look like this:

```json
{
  "output" : "/template.pot",
  "displaySrc" : true,
  "modules" : [
    {
      "name" : "json",
      "data" : {
        "files" : [
          {
            "paths" : ["/data"],
            "jsonPaths" : [
              "items:name"
            ]
          }
        ]
      }
    }
  ]
}
```

The `jsonPaths` elements are separated by `:`. It doesn't matter what type they
are (if array, object, etc.). As long as strings are at the end, you will get
translation keys from them.

For a more detailed example, please refer to the `examples/json` folder.

### SQLite

Databases are a little bit more complex, but this should still be as
straightforward as possible.

```json
{
  "output" : "/template.pot",
  "displaySrc" : true,
  "modules" : [
    {
      "name": "sqlite",
      "data" : {
        "dbs" : [
          {
            "path" : "/data.db",
            "tables" : [
              {
                "tablename" : "examples",
                "columns" : ["name"]
              }
            ]
          }
        ]
      }
    }
  ]
}
```

Every database file gets its own object inside the `dbs` array. You have to
define the path to the database file and the `tables`, that it shall take a look
into.

It will make a `SELECT` on all given table `columns` and expects them to be
`TEXT` aka translation keys.

## How to run

Call **potcreator** from your terminal and provide it the path to the directory
in which it can find the `.potcreator.json` file. For example:
`potcreator -p=path/to/my/folder`.

There are some minor flags that you can turn on. More info on those you can get,
if you call the tools help function: `potcreator -h`.

## How it works

1. Collect translation keys from the sources.
2. Merge duplicates, so that every given string is only once in the `*.pot` file later.
3. Create the `*.pot` file from the collected translation keys.

## Example usage

Check out the examples folder, to see some of potcreator's possibilities.

## License

potcreator is available under the MIT license. For more info, check out the
`LICENSE` file.

## How to contribute

* **Create a pull request:**
  * Feel free to add features, bug fixes etc. directly as pull request. As long
    as the features fit to the goal of this tool (making it easy to generate
    `*.pot` files from different sources and merge them together), and nothing
    breaks, the likelihood of getting your pull request accepted is quiet high.
  * Please try to stick with the projects code style.
  * Use [uncrustify](https://github.com/uncrustify/uncrustify) after you made your changes.
* **Create an issue:**
  * Found a bug / have a problem?
    * Give minimum information about where you are running potcreator (Windows, Linux, Mac)
    * What did you try so far? What exactly is not working?
  * Feature requests
    * As said in the beginning of this readme, I do not actively work on potcreator,
      but if you can't add the feature yourself, maybe someone else can, so please
      create an issue and describe what you would like to have and why you would
      like to have it.
  * Space for improvements
    * If you see something, that could be improved, feel free to create a ticket.

[^1]: https://en.wikipedia.org/wiki/Gettext#Programming
