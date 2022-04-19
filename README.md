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

# How to use
1. Create adv executable (run ```make build```)
2. Run adv \<filepath>

<!-- Check out the [examples](examples)! -->
