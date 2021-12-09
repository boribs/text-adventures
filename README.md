# Text Adventures!

A very simple text adventures engine that I developed for school.

It reads files that follow this syntax:
```
#Title
#Author
#Version

<section id> #section text
[
    <section id> #option text
    <section id> #option text
]
<section id> #section text
[
    <section id> #option text
    <section id> #option text
]
```

Check out the [examples](examples)!

As of 08/12/21 (dd/mm/yy), support for unicode was added!
The UTF-8 single-header librery comes from [here](https://github.com/sheredom/utf8.h).
