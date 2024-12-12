import numpy as np

def gaia_estimate_bv(Gs: np.ndarray, GBPs: np.ma.MaskedArray, GRPs: np.ma.MaskedArray):
    # See Table 5.9 of
    # https://gea.esac.esa.int/archive/documentation/GDR3/Data_processing/chap_cu5pho/cu5pho_sec_photSystem/cu5pho_ssec_photRelations.html

    Gcol = (GBPs - GRPs)
    if isinstance(GBPs, np.ma.MaskedArray):
        Gcol = Gcol.filled(0)
    if np.isnan(Gcol):
        Gcol = 0.
    Bs = Gs - 0.01448 + (0.6874 + (0.3604 + (-0.06718 + 0.006061 * Gcol) * Gcol) * Gcol) * Gcol
    Vs = Gs + 0.02704 + (-0.01424 + (0.2156 - 0.01426 * Gcol) * Gcol) * Gcol
    return Bs, Vs

def distance_grid(queries: np.ndarray, candidates: np.ndarray) -> np.ndarray:
    """
    Computes distances between arrays of celestial points

    queries: N x 2 array of (RA, Dec) of query points
    candidates: M x 2 array of (RA, Dec) of candidates (M >= N usu.)

    Returns an array of size N x M with distances
    """
    radec_pairs = np.concatenate([ # This incantation produces (ra1, dec1, ra2, dec2)
        np.broadcast_to(queries, (candidates.shape[0], queries.shape[0], 2)).transpose(2, 1, 0),
        np.broadcast_to(candidates, (queries.shape[0], candidates.shape[0], 2)).transpose(2, 0, 1)]
    ).transpose(1, 2, 0).reshape(-1, 4) # The first index is a flattened combination of (queries[i], candidates[j])

    distances = pykstars.CoordinateConversion.angular_distance(
        radec_pairs[:, 0],
        radec_pairs[:, 1],
        radec_pairs[:, 2],
        radec_pairs[:, 3],
    ).reshape(len(queries), len(candidates))

    return distances


