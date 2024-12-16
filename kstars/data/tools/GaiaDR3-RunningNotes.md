# What this file is

This file is for me (Akarsh Simha) and others in the future to keep track of the thinking and processing decisions that went into generating the next generation of the KStars star database.

# Reading data from binary star data files

The `stardataio` module in this directory can read KStars binary files in a convenient way

Eg:

```
from stardataio import KSStarDataReader
ksbin = KSStarDataReader('/usr/share/kstars/namedstars.dat')
print(ksbin[0]) # Prints `Trixel(id=0, count=0, offset=6458)`

print(ksbin[79][1]) # Prints `Record(offset=15290, {RA=6.903166, Dec=-12.03863, dRA=-138.9, dDec=-15.0, parallax=17.0, HD=5077.8, mag=4.23, bv_index=1.71, spec_type=K4, flags=, unused=})`
```

Note that RA is in decimal hours. There is an obvious error where the Henry-Draper number is divided by 10, because the scale is incorrectly encoded as 10 in the file header.

Essentially the binary star data dumps are indexed by trixel and then sorted by magnitude within each trixel

# `gaia_downloader.py`

This script (takes a very long time and) downloads about 300 million stars from the Gaia DR3 and puts them in Python pickles. It ensures each ADQL (Astronomy's version of SQL) query does not exceed the server limit and chunks it into magnitude bands. The pickles are therefore downloaded in magnitude order but it's best not to assume that each pickle is sorted as there is no `ORDER BY` in the ADQL query


# `hipparcos_to_db.py`

This script reads a bunch of catalogs and official cross-matches, also
tries to do its own cross-matches, and puts them in a SQLite
database. The following tables are generated. Look at the `metadata`
table for sources.

The script maintains [HTM indices](https://en.wikipedia.org/wiki/Hierarchical_triangular_mesh) of two different levels – the source data's indexer level and HTM-level 6 which has 8 * (4 ^ 6) = 32768 trixels. The typical source dataset has 512 trixels (level 3).

## Mostly Raw data

**Official Cross-Matches:**

* `hd_tyc2`: VizieR's official Henry-Draper/Tycho2 Cross Match. Note the issue with multiplicity: sometimes stars are resolved in HD but not in Tycho2.
* `tyc2_gaiadr3`: ESA's official Tycho2/GaiaDR3 Cross Match. Note the [xm_flag](https://gea.esac.esa.int/archive/documentation/GDR3/Catalogue_consolidation/chap_crossmatch/sec_crossmatch_algorithm/) can indicate sources which are duplicated in Tycho2.

**Official Catalogs:**

* `hipparcos`: Hipparcos data downloaded from VizieR
* `athyg`: AT-HYG from [here](https://www.astronexus.com/hyg). This is a cross-match of various catalogs, and uses Gaia DR3 data. So whatever we produce can be benchmarked against it for bright stars.
* `tycho2`: Tycho2 downloaded in chunks from VizieR and combined together
* `tycho1`: Tycho1 downloaded from VizieR

`tcho2`, `tycho1` and `hipparcos` have a measurement epoch of J1991.25 but the coordinates are still referred to the equinox of J2000, i.e. the ICRS frame for all practical purposes. We store the original J1991.25 coordinates as `epra` and `epdec`, but we also propagate the proper motions to J2000.0 and store them as jra and jdec when the proper motions are available. So it's best to use `COALESCE(jra, epra)` etc. to get the best estimate for J2000 equinox/epoch coordinates.

**KStars Binary Data:**

KStars binary data has mixed up epoch, a bug! Massimiliano Maserati first reported this on kstars-devel, and although we thought it was patched, the patch was made to the ASCII file, not the binary dumps. So this error persists in the present version. This means that the `ksbin` table has (`ra`, `dec`) referred correctly to the J2000 equinox (practically ICRS), but the epoch may be either J1991.25 or J2000.0 depending on which catalog it came from. The data is J1991.25 if it came from Tycho-1 but J2000.0 if it came from Tycho-2 (*TODO: Check this claim*)

KStars binary data also has duplicates star records to account for proper motion: the same star record is written into all trixels that it crosses in ±10,000 years from J2000.0. The easiest way to identify a proper motion duplicate is to find the trixel corresponding to its (ra, dec) and if the trixel is different from the trixel into which the star record was written, it must be a proper-motion copy.

The script reads all three Tycho-based star data files – named, unnamed and deep – into a single table, `ksbin`. It does not care for USNO-NOMAD-1e8.dat, the 100M-star catalog. It also reads `starnames.dat` which maps the names of the named stars. Note that `ksbin_id` in the `starnames` table refers to the row id (i.e. FOREIGN KEY) in the `ksbin` table. This is because the KStars binary data doesn't really provide a unique sequential ID for every record – a star record is instead uniquely identified by the tuple `(file, trixel, tr_index)` (or by `(file, offset)` but this is not desirable since the offsets are not sequential).

## Post-processing

### `pm_duplicates` and `ksbin_nodups`

The `pm_duplicates` table contains `ksbin_id`s (`id` field from the `ksbin` table). Note that there can be multiple `dup_id` rows for each `orig_id`, since the star may be duplicated in multiple trixels.

`ksbin_nodups` is a view where the duplicate rows are removed

### Cross-match of KStars binary data with Tycho-2

In generating the cross-match with Tycho-2 we compare angular distance from `COALESCE(jra, epra)/COALESCE(jdec, epdec)` with the (`ra`, `dec`) values in `ksbin`. There's a note that in binary systems, the `jra`/`jdec` are collapsed to the same value for both stars, so we disambiguate using magnitudes to map the components correctly. If the magnitudes are too close, then `epra` and `epdec` fields are used instead.

The rationale for this has to do with our best guess for how the
catalog was constructed. Looking at old e-mails from Jason Harris who
did the cross-match, the Tycho2 assignments were made with a 1" cone
search, and if that did not match, a 5" cone search with a magnitude
difference constraint of 0.2. Thus, if a Tycho2 JRA supplanted one of
the two stars in a binary, it must be the one whose J1991.25 was
closer to J2000. Jason did account for proper motion, so I'm not
exactly sure when and where the epochs got mixed up; perhaps this was
in my ingestion of Tycho1 and Tycho2 notwithstanding their epochs

The output is written to the table `ks_tyc2`.


### Cross-match of KStars binary data with Hipparcos/Tycho-1

In generating the cross-match with Tycho-1/Hipparcos, we assume that both files contain ep=J1991.25 ICRS coordinates and use only angular distances to find the best match. The results are written into tables `ks_tyc1` and `ks_hip`.

### Overall cross-match of KStars binary data (`ks_xm`)

We create the `ks_xm` table by looking for either Tycho-1 or Tycho-2 matches within 1e-5 degree (=0.036") distance, whichever is better. Fortunately, this results in matching *all* stars with a maximum error of 1e-5 degrees. The largest differences (of the order of 1e-5) are on Tycho-2 stars. I don't understand why, perhaps because of differences in epoch propagation or in floating-point computations.

In any case _all_ 2451251 stars in the KStars catalog are matched with such tight error bounds to the source Tycho-1 or Tycho-2 catalogs. This can be verified by running the following SQL query:

```
select count(*) from ksbin_nodups left join ks_xm on ks_xm.ks_id = ksbin_nodups.id where ks_xm.ks_id is null
```

Thus, by looking at the `ks_xm` tables, we can identify the source catalog of every KStars star, and therefore know the actual epoch it belongs to!

### Multiple matches from KStars to Tycho-1/Tycho-2

Mysteriously, KStars' binary files seem to have multiple objects that match the same Tycho-1/Tycho-2 designations. The following query sheds light:
```
select ks_xm.*, ks_tyc1.xm_id as TYC1, ks_tyc2.xm_id as TYC2, ksbin_nodups.* from ks_xm
inner join (select ks_xm.TYC, count(*) as num from ks_xm group by ks_xm.TYC having num > 1)
using(TYC)
inner join ks_tyc1 using(ks_id)
inner join ks_tyc2 using(ks_id)
inner join ksbin_nodups on ksbin_nodups.id = ks_xm.ks_id
```

For example, here is a result from this query: KStars data files seem to have an artifact string of stars near the star TYC 1040-324-1. It is unclear where this comes from.

### Unmatched Tycho-1 stars

The following query lists 1837 unmatched Tycho-1 stars:
```
select * from tycho1
left join ks_tyc1 on ks_tyc1.xm_id = tycho1.TYC
where ks_tyc1.xm_id is NULL
```

However, many of them have null magnitude and were probably discarded as a reason. Instead when we run this query:
```
select * from tycho1
left join ks_tyc1 on ks_tyc1.xm_id = tycho1.TYC
where ks_tyc1.xm_id is NULL
and coalesce(tycho1.V, tycho1.BT, tycho1.VT) < 12.0
```
we only find 72 rows.

Spot checking a couple of these, though, I _did_ find the stars plotted in KStars, and they're matched against Tycho2 with different last-digit in the TYC designation.

### Unmatched Tycho-2 stars
A similar query with Tycho-2:
```
select * from tycho2
left join ks_tyc2 on ks_tyc2.xm_id = tycho2.id
where ks_tyc2.xm_id is NULL
and coalesce(tycho2.BT, tycho2.VT) < 12.0
```
returned 7694 rows! I spot checked two of these; one was present in KStars, the other was not!

This indicates that there are bona fide Tycho-2 stars missing from KStars catalog, not sure why. TYC 141-815-1 is an example.

### Why cross-match the old catalogs?

This way, we can ensure that we have the data from Tycho-1/Tycho-2 that Gaia DR3 might not have. For example, the mapping of star names, and the Tycho BT/VT bands are different from the Gaia G, G_BP, G_RP magnitude bands. If I remember correctly, Gaia DR3 is not complete in the bright stars. Thus we would like to retain the good data from Tycho-2/Tycho-1 and only supplant the Tycho-1/2 astrometry with the new astrometry from Gaia DR3.

# Looking forward

I think it is best to regenerate all catalogs from the official data and using official cross-matches wherever possible. Therefore it is worth analyzing what each catalog provides us with.

## Henry-Draper / Tycho-2 cross-match

The [Henry-Draper / Tycho-2 cross-match paper](https://www.aanda.org/articles/aa/pdf/2002/17/aah3397.pdf) states the following:

> The identiﬁcation list gives Tycho-2 identiﬁcations for 353 473 HD
> stars, cf. Table 1. For the majority, 352 957 stars, it is a one to
> one identiﬁcation, while 253 Tycho-2 stars are identiﬁed with two HD
> stars each, and 10 HD stars are resolved in Tycho-2. All these cases
> are ﬂagged in the list, which also gives the spectral type and a
> double star ﬂag for stars in the Tycho Double Star Catalogue
> (Fabricius et al. 2002).

The concerning part is the 253 double-matches, and we can see if the Gaia DR3 cross-match sheds any light on these:
```
select * from hd_tyc2
join (select hd_tyc2.TYC, COUNT(*) as num from hd_tyc2 group by hd_tyc2.TYC having num > 1) using(TYC)
join tyc2_gaiadr3 using(TYC)
```
The query returns 258 (not 253??) rows, all having `num_neighbors` of 1 and having `xm_flag` values of 8, 16 and 72. None of these indicate resolution into a double star.

*Therefore, we will treat the case `n_HD > 1` as a duplicate Henry-Draper identification for the same Tycho2 star*

Now for the case of the `n_TYC > `, i.e. multiple Tycho2 stars for the same HD identifier. I made two spot checks on SIMBAD: in one case SIMBAD collapsed the identifier to one of the two stars. In another case, it listed the object as a double / multiple star with two child objects. BTW, there were 15 rows returned, not 10 as the paper claimed.

*To handle this case, we can assign the HD number to both of the stars. In the `Henry-Draper.idx` index file, though, we can pick one of the two stars. This should not cause much problem as the user will find both stars next to each other. One must remember that HD designations are thereby not unique.*


## Hipparcos, Tycho-1 and Tycho-2

The full documentation for the products of the ESA Hipparcos mission, viz. catalogs Hipparcos, Tycho-1 and Tycho-2, are [here](https://vizier.cfa.harvard.edu/ftp/cati/more/HIP/cdroms/docs/). Tycho-2 has a [homepage here](https://www.astro.ku.dk/~erik/Tycho-2/) and documentation [here](https://gea.esac.esa.int/archive/documentation/GDR1/datamodel/Ch4/tycho2.html)

Tycho-2 fully supercedes Tycho-1, so there isn't much value in retaining Tycho-1 other than to have Hipparcos magnitudes. In that event, a Hipparcos cross-match can be made in any case.


An old email from Jason Harris lists several issues with Tycho-2:

```
Some problems with the Tycho-1 (1 million stars) and Tycho-2 (2.5
million stars) catalogs:

+ Tycho-2 does not report parallax data
+ Tycho-2 does not include the brightest stars
+ Neither Tycho-1 nor Tycho-2 report the spectral type
+ Tycho-2 doesn't report variability or multiplicity
+ Tycho-1 does not report the delta-V or periodicity of variable
stars, it only reports whether a star is variable or not.
+ While most of the data is in exquisite agreement with the hipparcos
data we derived stars.dat from, I noticed one disturbing discrepancy
already: Tycho puts the V magnitude of Arcturus at +0.16, rather than
-0.05 as in Hipparcos (and most online references).  How can it be 0.2
mag off on the 3rd brightest star in the sky???
+ Many of the brightest stars don't have parallax and proper motion
data in Tycho, whereas the same stars do have these data in Hipparcos.
```

He further went on to say that we'd need to merge the Hipparcos and
Tycho-1 catalogs together to get all the data we want for the
brightest 120000 stars. We also had decided to "fake" the spectral
type from the B-V color, which is what we'dve done for USNO NOMAD as
well.

Many of the problems reported above are fixed by the cross-matching of
AT-HYG. However, AT-HYG does not have about 6000 stars that are in
Tycho-2 (why?). There are only 25 stars in Tycho-2 that do not have V
magnitudes but only possess B magnitudes. Of these, seven stars are
not matched in Gaia DR3 / Tycho2 (`tyc2_gaiadr3` table) and the rest
seem to be bonafide. It looks like the best approach is to use
Tycho2/TDSC-Merge as the primary database for bright (V < 11.5) stars
and cross-reference AT-HYG for the other data

Also it appears that the AT-HYG code is not open source (at least I
couldn't find it), only the catalog is released. We can try contacting
the author if needed. I however believe there isn't much "risk" in
trusting the catalog, (a) it is unlikely that we can do better as the
author has done both positional and magnitude cross-matching, and (b)
we have positional data from official sources (Tycho-2/TDSC)

## Tycho-2 / TDSC Merge

Here is [info](https://gea.esac.esa.int/archive/documentation/GDR3/Catalogue_consolidation/chap_crossmatch/sec_crossmatch_externalCat/ssec_crossmatch_tdsc.html) on the catalog, as well as [details on the table](https://gea.esac.esa.int/archive/documentation/GEDR3/Gaia_archive/chap_datamodel/sec_dm_external_catalogues/ssec_dm_tycho2tdsc_merge.html). The script downloads the data by way of ADQL query. Luckily, all 2.5M stars are returned in a single query with no need to break it into chunks.

The only thing the script computes on top of the downloaded data is the epoch=J2000 ICRS (RA, Dec) propagated by applying proper motion, if the catalog does not provide it, and the HTM trixel for this (ra, dec) at the target indexer level (6). Thus the `ra` and `dec` fields in the table are filled with the indexed coordinates which are either of the following in the given order of precedence:

* The mean RA/Dec at Epoch J2000.0 given in the catalog, i.e. `ra_mdeg` and `de_mdeg` fields
* The coordinates propagated by us from the observation epoch using the `pm_ra` and `pm_de` values, where the observation epoch used is the average of `ep_ra_m` and `ep_de_m`, or J1991.25 if not provided in the catalog
* The observed coordinates `ra_deg` and `de_deg` at the observation epoch if no proper motion values are provided

## Analysis of Tycho-2 / TDSC Merge

There are only 20 stars with no VT-magnitude. Of the 20, only 3 do not exist in Gaia DR3 cross-match (`tyc2_gaiadr3`). It also appears that there are 17404 entries in `tycho2tdsc_merge` that are not present in `tycho2`, and 12764 rows in `tycho2` that don't join `tycho2tdsc_merge` on the `TYC` identifier. This is probably because `tycho2tdsc_merge` supercedes them.

Since this was the catalog used to merge with Gaia DR3, there are no entries in the `tyc2_gaiadr3` table that are present in the `tycho2tdsc_merge` table. However, there are 43,557 stars in `tycho2tdsc_merge` that are not in `tyc2_gaiadr3`. Why?

I ran the following query to spot check a few stars:
```
select tycho2tdsc_merge.ra/15.0 as rah, tycho2tdsc_merge.dec as decd, * from tycho2tdsc_merge
full join tyc2_gaiadr3 using(TYC)
where tyc2_gaiadr3.TYC is null
order by rah
```

Most of the coordinates pointed to bona fide stars. Sometimes there were unresolved double stars seen on the DSS2. Sometimes there were two stars plotted in KStars binary files at the location. In any case, to fully understand what's going on, we'll need to process Gaia DR3.

I initially did this analysis with AT-HYG v2.4, which the script would put in the `athyg` table. I modified the script to put v3.2 which is current as of Dec 2024 into a table `athg32` and re-did the analysis:

Now for the match against AT-HYG. Firstly AT-HYG returns 981 rows with no `TYC` field, above 10.2 mag. This is not bad, so almost every star in AT-HYG has been identified with the Tychos. Now we check the cross-match against `tycho2tdsc_merge`,
```
select COALESCE(tycho2tdsc_merge.ra, athyg.ra)/15.0 as rah, COALESCE(tycho2tdsc_merge.dec, athyg.dec) as decd, * from tycho2tdsc_merge
full join athyg using(TYC)
where tycho2tdsc_merge.TYC is null
order by rah
```
Spot checking a few of the rows returned, we find that there are valid stars on the DSS2 at these coordinates. BTW, the brightest star that did not match is Canopus, followed by Achernar!

After we build the `tycho2tdsc_merge` into binary files and render it in KStars, we must check that the stars in AT-HYG and the stars are present in the shallow binary files, and also see what happens regarding the Gaia DR3 stars. Since GDR3 is only anticipated to be [complete from G=12 to G=17 mag](https://gea.esac.esa.int/archive/documentation/GDR3/Catalogue_consolidation/chap_cu9val/sec_cu9val_introduction/ssec_cu9val_intro_completeness.html), we should retain all `tycho2tdsc_merge` stars in our shallow catalog irrespective of whether they are present in the GDR3 merge or not.

In any case it looks like this is the state-of-the-art catalog we wish to develop into the KStars binary files: we generate this catalog, using AT-HYG to get the data that is missing in `tycho2_tdscmerge`. We then process GDR3 as well, excluding the stars that were cross-matched with `tycho2tdsc_merge`. We then spot-check to see if we caught all the AT-HYG stars that did not match against `tycho2tdsc_merge`, and also spot-check that the GDR3 did not introduce duplicates. This would be made easier once the catalogs are compiled and rendered in KStars.

## How to combine `tycho2tdsc_merge` and `athyg`

1. We will `left join` on `tycho2tdsc_merge`
2. *Position*: Tycho-2 shockingly has no proper motion on Proxima Centauri (and Lacaille 8760), but it looks like AT-HYG v3.2 has problems too: it interprets Tycho2 RA/Dec values which are at observation epoch as ep=J2000.0 values. These are made evident by the following query:
```
SELECT athyg32.ra as athyg_ra, athyg32.dec as atyhg_dec, tycho2tdsc_merge.ra as tyc_ra, tycho2tdsc_merge.dec as tyc_dec, tycho2.jra as tyc2_ra, tycho2.jdec as tyc2_dec, tycho2tdsc_merge.ra - athyg32.ra as err_ra, tycho2tdsc_merge.dec - athyg32.dec as err_dec, tycho2tdsc_merge.pmra as t2t_pmra, tycho2tdsc_merge.pmdec as t2t_pmdec, *
from tycho2tdsc_merge
full join athyg32 using(TYC)
full join tycho2 using (TYC)
order by err_ra desc
limit 200
```
To confirm that my interpretation of the [`tycho2tdsc_merge` fields](https://gea.esac.esa.int/archive/documentation/GEDR3/Gaia_archive/chap_datamodel/sec_dm_external_catalogues/ssec_dm_tycho2tdsc_merge.html) was not mistaken, I spot-checked one of the stars (HIP 1068) against SIMBAD. SIMBAD claims to report ep=J2000 ICRS coordinates, and the `ra`/`dec` I calculated was off from SIMBAD's value by only 0.2", whereas the AT-HYG value was off by 18".

So it looks like the most reliable positions will come from taking Tycho-2 positions, and when necessary (i.e. no mean J2000.0 position is given in Tycho-2) we should propagate the proper motion using the proper motions coalesced from AT-HYG. We can later bring in Gaia DR3 positions and propagate back from 2016.0 with Gaia proper motions which would be more accurate.

3. *Magnitude*: Going from Jason Harris' comment above and looking into SIMBAD's magnitudes as well, it looks like Tycho-2 should not be relied upon for magnitudes when possible. So it appears that the best magnitudes come from AAVSO Photometric All-Sky Survey (APASS) which are incorporated in UCAC4. Neither UCAC4 or AAVSO provide cross-match ids with Tycho-2. A cross-match between [Gaia DR3 and APASS](https://gea.esac.esa.int/archive/documentation/GEDR3/Catalogue_consolidation/chap_crossmatch/sec_crossmatch_externalCat/ssec_crossmatch_apass.html) exists which can be used to get higher-quality Johnson-Cousins photometry.

My conclusion is that the preference for Johnson-Cousins B and V magnitudes should be as follows:

APASS > Hipparcos > Estimate from Tycho-2 BT/VT > Estimate from Gaia DR3 G/GBP/GRP

I base this on the following notes:
* APASS is true Johnsons B/V although it is ground-based photometry; It was included in UCAC4 by the USNO.
* Hipparcos combines many sources to get a spectral-type-informed best estimate of Johnsons B/V, and many of the time, the source is ground-based (`WHERE hipparcos.mag_src = "G"`), see [documentation](https://heasarc.gsfc.nasa.gov/w3browse/all/hipparcos.html) for the `Vmag_Source` field.
* Tycho-2 BT and VT filters are close to B and V filters although not perfect
* The spread of error in G/GBP/GRP estimates of B and V is quite large -- see [here](https://gea.esac.esa.int/archive/documentation/GEDR3/Data_processing/chap_cu5pho/cu5pho_sec_photSystem/cu5pho_ssec_photRelations.html)

*Question:* Should we prefer Hipparcos' `H` and `T` estimates over Tycho-2? I think the answer is yes, we should defer to Tycho-2 only when the star is not present in Hipparcos, because of the information [presented on Page 44, Paragraph 1 here](https://vizier.cfa.harvard.edu/ftp/cati/more/HIP/cdroms/docs/vol1/sect1_03.pdf) – Hipparcos uses spectral type to estimate the color index, I presume the same is used to estimate V mag from Tycho photometry.

*Note:* AT-HYG's magnitudes are either Hipparcos' "V" or Tycho-2's VT. I tried to see if the choice was related to the Hipparcos' [original magnitude source](https://heasarc.gsfc.nasa.gov/w3browse/all/hipparcos.html) (`hipparcos.mag_src`) being "Ground" or "Hipparcos", but I could not infer the pattern. When Tycho2 is used, no conversion formula from VT to V is applied

All of these except for APASS are easy to combine since there are cross-identifications. For APASS, we will either have to build a cross-match or use [the cross-match performed for Gaia DR3](https://gea.esac.esa.int/archive/documentation/GEDR3/Catalogue_consolidation/chap_crossmatch/sec_crossmatch_externalCat/ssec_crossmatch_apass.html). Since the cross-match is for photometry, we will be conservative in our cross-matching and accept false negatives rather than have false positives

4. *Color Index*: Hipparcos B-Vs are more accurate than Tycho-2 due to the use of actual Johnson-Cousins photometry or spectral-type-informed estimates when available, see [this document](https://vizier.cfa.harvard.edu/ftp/cati/more/HIP/cdroms/docs/vol1/sect1_03.pdf) Page 44, first paragraph. By the same rationale, I would take the same preference order used for magnitude in approaching color index

5. *Spectral Type*: We pick from AT-HYG. Since KStars only supports 2-character spectral types, we will pick the first two characters from AT-HYG's spectral type. This is probably from Hipparcos.

5. *Proper Motion and Parallax*: Prefer AT-HYG since it incorporates Gaia data and a thorough job has been done by the author in filtering the data well, [see Changelog](https://github.com/astronexus/ATHYG-Database/blob/main/version-info.md).

6. *Names*: We should pick names from AT-HYG, it seems most thorough and definitely more thorough than our existing `starnames.dat`. We should consider picking up Flamsteed designations.


# Writing star data

## Tooling

The `stardataio` module has several writers to write stars to the disk. All of them assume that you have the stars pre-sorted in magnitude order, but not necessarily trixel order. It appears from my re-reading of the code that `KSStarDataWriter` expects the trixels in order, but the very high-level `KSBufferedStarCatalogWriter` does not – the latter just expects you to stream the data in decreasing order of brightness and it takes care of indexing and distributing across trixels, including handling proper-motion duplication for the specified number of years.

I also introduced `v2` replacements for the `DeepStarData` and `StarData` records that are identical in structure and memory (and therefore can be used with exactly the same data structures), but changes the scale as the proper motion scale is otherwise too limiting; this is however a backwards-incompatible change and requires restructuring some of the code. We should decide if this is worth the effort (probably is). It might also be worth considering new data structures that carry more data, such as Flamsteed numbers. I believe Flamsteed numbers will be very helpful in KStars

## Writing `tycho2tdsc_merge`

# Gaia DR3

Caveats:

* https://www.cloudynights.com/topic/770966-the-use-of-accurate-star-catalogues/?p=11099363
* https://www.cosmos.esa.int/web/gaia/dr3-known-issues
* https://www.cosmos.esa.int/web/gaia/dr3-data-gaps
* An obvious caveat of Gaia DR3 is the G/GBP/GRP photometry
