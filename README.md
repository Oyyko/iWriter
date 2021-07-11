# iWriter
a markdown writer, which can be used to generate static html website. Also it can be used to manage USTC homepage.

## install
`cd .../iWrirer`
`g++ main.cpp -o iWriter`

## usage
`./iWriter -help`

Then you will see

```sh
Usage:
    trans [<option>...] [input-file]

Available options are:
    --help      Show help.
    --set aaa bbb 
               Set your username as aaa, password as bbb
```

For example, 
`./iWriter --set john 123456` means setting the `username` as `john` and setting the `password` as `123456`

`./iWriter doc.md` means translate `doc.md` as `index.html` and then upload it.