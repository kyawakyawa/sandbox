# Apply Sim3

## 説明
- 1フレーム目
カメラ座標Aでの可視化

- 2フレーム目
カメラ座標Bでの可視化

- 3フレーム目
カメラ座標Aの姿勢に対し、相対Sim3を適用してカメラ座標Bに補正したもの。  
なお、Sim3はカメラ座標Aからカメラ座標Bに適用するものであり、
カメラ座標Aはlocal to world形式で表現されている。

$$T_{l2w,fixed}=S_{A to B} \cdot T_{l2w}$$

ここで  
$$ 
T_{l2w,fixed}=
\begin{pmatrix} 
    sR_{l2w,fixed} & \mathbf{t_{l2w,fixed}} \\
    \mathbf{0} & \mathbf{1}
\end{pmatrix}
$$
との形となり、回転成分に余計なスケールが付く。  
結論としてはスケールを外してしまえばよい。
$$ 
T^{'}_{l2w,fixed}=
\begin{pmatrix} 
    R_{l2w,fixed} & \mathbf{t_{l2w,fixed}} \\
    \mathbf{0} & \mathbf{1}
\end{pmatrix}
$$

この $T^{'}_{l2w,fixed}$ を新しい姿勢とすればよい。

- 4フレーム目
カメラ座標Aをworld to localとして3フレーム目と同じ計算をした例。  
当然だがうまく行かない