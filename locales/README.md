# How to contribute a new narrative language file

1. Copy the en-US.json to `<NEW_LANGUAGE_TAG>.json`
Using [IETF BCP 47](https://tools.ietf.org/html/bcp47) as reference - the typical format for the `<NEW_LANGUAGE_TAG>` is:  
<[ISO 639 two-letter language code](https://en.wikipedia.org/wiki/List_of_ISO_639-1_codes)>-<[ISO 3166 two-letter country code](https://en.wikipedia.org/wiki/ISO_3166-1_alpha-2)>  
Czech/Czech Republic example:  
`cs-CZ`

2. Update the `posix_locale` value in your new file. The character encoding must be UTF-8. The typical format is:  
<[ISO 639 two-letter language code](https://en.wikipedia.org/wiki/List_of_ISO_639-1_codes)>_<[ISO 3166 two-letter country code](https://en.wikipedia.org/wiki/ISO_3166-1_alpha-2)>.UTF-8  
Czech/Czech Republic example:  
`cs_CZ.UTF-8`

3. Do not translate the JSON keys or phrase tags. An example using the ramp instruction:  
![Alt text](/locales/img/do_not_translate.png)

4. Please translate the JSON values. As needed, reorder the phrase words and tags - the tags must remain in the phrase. An example using the ramp instruction:  
![Alt text](/locales/img/translate.png)

5. Run `make check` on the odin repo to verify the tests pass OR move on to step#6 and we can help verify.

6. Submit a pull request for review. Thank you!

#### Instruction descriptions
`TODO`

#### Tag descriptions
`TODO`

