# Text Adventures!

A very simple text adventures engine that I developed for school.

It reads files that follow this syntax (JSON):
```
{
"title": "adventure's title",
"author": "adventure's author",
"version": "adventure's version",
"sections": [
    {
        "id": #section id,
        "text": "section text",
        "options": [
            {
                "id": #option id,
                "text": "option text"
            }
        ]
    }
]
}
```

Check out the [examples](examples)!
