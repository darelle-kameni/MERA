#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Script d'entraînement CORRIGÉ - VERSION STANDALONE
Sauvegardez ce fichier comme: train_fixed.py
"""

import os
# IMPORTANT: Définir la variable avant d'importer mindspore
os.environ['PROTOCOL_BUFFERS_PYTHON_IMPLEMENTATION'] = 'python'

import numpy as np
import mindspore as ms
import mindspore.nn as nn
from mindspore import context, Tensor
from mindspore.train.callback import Callback, ModelCheckpoint, CheckpointConfig
from mindspore.train import Model
import mindspore.dataset as ds
from mindspore.dataset import vision, transforms
import mindspore.dataset.vision.py_transforms as py_vision

context.set_context(mode=context.GRAPH_MODE, device_target="CPU")

# Paramètres OPTIMISÉS
DATASET_PATH = "./Master_Eye_Dataset"
BATCH_SIZE = 32
NUM_EPOCHS = 150
LEARNING_RATE = 0.0001  # RÉDUIT
IMAGE_SIZE = 224
NUM_CLASSES = 7

CLASSES = {
    0: "Sain",
    1: "Cataracte",
    2: "Conjunctivite",
    3: "Jaunisse",
    4: "Pterygion",
    5: "Blepharite",
    6: "Uveitis"
}


# ============================================================================
# ARCHITECTURE DU MODÈLE (TRANSFER LEARNING AVEC RESNET-50)
# ============================================================================
# Importer ResNet-50 depuis mindvision (compatible avec MindSpore 2.7.1)
from mindvision.classification.models import resnet50


class EyeDiseaseNet(nn.Cell):
    """
    Réseau de neurones utilisant le transfert d'apprentissage avec ResNet-50.
    Ce code est maintenant compatible avec les modèles de mindspore.vision et mindvision.
    """
    def __init__(self, num_classes=7, pretrained=True):
        super(EyeDiseaseNet, self).__init__()
        
        # Charger ResNet-50 pré-entraîné
        self.backbone = resnet50(pretrained=pretrained)

        # Le nombre de caractéristiques en entrée de la couche finale de ResNet-50 est 2048.
        # Nous utilisons cette valeur directement pour une robustesse maximale.
        in_channels = 2048

        # Remplacer la dernière couche, en s'adaptant au nom ('fc' ou 'head')
        if hasattr(self.backbone, 'fc'):
            self.backbone.fc = nn.Dense(in_channels, num_classes)
            print("    -> Couche finale 'fc' remplacée.")
        elif hasattr(self.backbone, 'head'):
            self.backbone.head = nn.Dense(in_channels, num_classes)
            print("    -> Couche finale 'head' remplacée.")
        else:
            raise AttributeError("Le modèle pré-entraîné n'a ni couche 'fc' ni 'head' à remplacer.")

    def construct(self, x):
        return self.backbone(x)


# ============================================================================
# FONCTION POUR CALCULER LES POIDS DES CLASSES
# ============================================================================

def compute_class_weights(dataset_path):
    """Calculer les poids de classes pour équilibrer l'entraînement"""
    print("\n🔍 Analyse de l'équilibre des classes...")
    
    class_counts = {}
    total = 0
    
    for class_idx in range(NUM_CLASSES):
        # Trouver le dossier
        class_folder = None
        for folder in os.listdir(dataset_path):
            if str(class_idx) in folder:
                class_folder = folder
                break
        
        if class_folder:
            class_path = os.path.join(dataset_path, class_folder)
            count = len([f for f in os.listdir(class_path) 
                        if f.lower().endswith(('.png', '.jpg', '.jpeg'))])
            class_counts[class_idx] = count
            total += count
            print(f"  Classe {class_idx} ({CLASSES[class_idx]:15s}): {count:4d} images")
        else:
            class_counts[class_idx] = 1
    
    # Calculer les poids inversement proportionnels
    weights = []
    for class_idx in range(NUM_CLASSES):
        weight = total / (NUM_CLASSES * class_counts[class_idx])
        weights.append(weight)
    
    # Normaliser
    weights = np.array(weights)
    weights = weights / weights.sum() * NUM_CLASSES
    
    print(f"\n📊 Poids de classes calculés:")
    for class_idx, weight in enumerate(weights):
        print(f"  {CLASSES[class_idx]:15s}: {weight:.3f}")
    
    return weights


# ============================================================================
# LOSS PONDÉRÉE
# ============================================================================

class WeightedLoss(nn.Cell):
    """Loss pondérée pour gérer le déséquilibre des classes"""
    
    def __init__(self, class_weights):
        super(WeightedLoss, self).__init__()
        self.class_weights = Tensor(class_weights, ms.float32)
        self.softmax_cross_entropy = nn.SoftmaxCrossEntropyWithLogits(sparse=True, reduction='none')
    
    def construct(self, logits, labels):
        loss = self.softmax_cross_entropy(logits, labels)
        weights = self.class_weights[labels]
        weighted_loss = loss * weights
        return weighted_loss.mean()


# ============================================================================
# CRÉATION DU DATASET
# ============================================================================

def create_dataset(data_path, batch_size=32, is_training=True):
    """Créer le dataset avec augmentation en utilisant les py_transforms plus stables."""
    
    dataset = ds.ImageFolderDataset(
        data_path,
        num_parallel_workers=2,
        shuffle=is_training
    )
    
    # Utilisation des py_transforms pour une meilleure stabilité
    if is_training:
        # Les augmentations qui fonctionnent bien sur les images PIL
        transform_pil = [
            vision.Decode(to_pil=True),
            vision.Resize((IMAGE_SIZE, IMAGE_SIZE)),
            vision.RandomHorizontalFlip(prob=0.5),
            vision.RandomVerticalFlip(prob=0.2),
            vision.RandomRotation(degrees=30),
            vision.RandomColorAdjust(brightness=0.3, contrast=0.3, saturation=0.3)
        ]
        # Les transformations finales qui convertissent en tenseur et normalisent
        transform_tensor = [
            py_vision.ToTensor(), # Met à l'échelle [0-255]->[0-1] et transpose HWC->CHW
            py_vision.Normalize(mean=[0.485, 0.456, 0.406], std=[0.229, 0.224, 0.225])
        ]
    else:
        transform_pil = [
            vision.Decode(to_pil=True),
            vision.Resize((IMAGE_SIZE, IMAGE_SIZE))
        ]
        transform_tensor = [
            py_vision.ToTensor(),
            py_vision.Normalize(mean=[0.485, 0.456, 0.406], std=[0.229, 0.224, 0.225])
        ]

    # Appliquer les transformations en deux étapes pour s'assurer que py_transforms reçoit le bon type
    dataset = dataset.map(operations=transform_pil, input_columns="image", num_parallel_workers=2)
    dataset = dataset.map(operations=transform_tensor, input_columns="image", num_parallel_workers=2)
    
    # Conversion du type de label et mise en lots
    dataset = dataset.map(operations=transforms.TypeCast(ms.int32), input_columns="label")
    dataset = dataset.batch(batch_size, drop_remainder=True)
    
    return dataset


# ============================================================================
# CALLBACK POUR MONITORING
# ============================================================================

class DetailedMonitor(Callback):
    """Callback pour surveiller l'entraînement"""
    
    def __init__(self, val_dataset, per_print_times=20):
        super(DetailedMonitor, self).__init__()
        self.per_print_times = per_print_times
        self.val_dataset = val_dataset
        self.losses = []
        self.best_acc = 0.0
        self.step_count = 0
        
    def on_train_step_end(self, run_context):
        cb_params = run_context.original_args()
        loss = cb_params.net_outputs
        
        if isinstance(loss, (tuple, list)):
            loss = loss[0]
        
        self.step_count += 1
        loss_value = float(loss.asnumpy())
        self.losses.append(loss_value)
        
        if self.step_count % self.per_print_times == 0:
            avg_loss = np.mean(self.losses[-self.per_print_times:])
            cur_epoch = cb_params.cur_epoch_num
            print(f"Epoch {cur_epoch:3d} | Step {self.step_count:4d} | Loss: {avg_loss:.4f}")
    
    def on_train_epoch_end(self, run_context):
        cb_params = run_context.original_args()
        cur_epoch = cb_params.cur_epoch_num
        
        # Évaluation rapide
        network = cb_params.network
        network.set_train(False)
        
        correct = 0
        total = 0
        class_correct = {i: 0 for i in range(NUM_CLASSES)}
        class_total = {i: 0 for i in range(NUM_CLASSES)}
        
        for batch_idx, (images, labels) in enumerate(self.val_dataset.create_tuple_iterator()):
            
            outputs = network(images)
            preds = np.argmax(outputs.asnumpy(), axis=1)
            labels_np = labels.asnumpy()
            
            correct += np.sum(preds == labels_np)
            total += len(labels_np)
            
            for i in range(len(labels_np)):
                true_label = labels_np[i]
                pred_label = preds[i]
                class_total[true_label] += 1
                if pred_label == true_label:
                    class_correct[true_label] += 1
        
        accuracy = (correct / total) * 100 if total > 0 else 0
        
        print(f"\n{'='*70}")
        print(f"EPOCH {cur_epoch:3d} TERMINÉ")
        print(f"{'='*70}")
        print(f"Loss moyenne: {np.mean(self.losses[-50:]):.4f}")
        print(f"Accuracy validation: {accuracy:.2f}%")
        
        print(f"\nAccuracy par classe:")
        for class_idx in range(NUM_CLASSES):
            if class_total[class_idx] > 0:
                class_acc = (class_correct[class_idx] / class_total[class_idx]) * 100
                print(f"  {CLASSES[class_idx]:15s}: {class_acc:5.1f}%")
        
        if accuracy > self.best_acc:
            self.best_acc = accuracy
            print(f"\n⭐ NOUVEAU RECORD: {accuracy:.2f}%")
        
        print(f"{'='*70}\n")
        
        network.set_train(True)
        self.losses = []


# ============================================================================
# FONCTION D'ENTRAÎNEMENT PRINCIPALE
# ============================================================================

def train_model():
    """Fonction principale d'entraînement"""
    
    print("\n" + "="*70)
    print("ENTRAÎNEMENT CORRIGÉ - DÉTECTION D'ANOMALIES OCULAIRES")
    print("="*70)
    
    # Calculer les poids
    class_weights = compute_class_weights(DATASET_PATH)
    
    # Créer datasets
    print(f"\n📁 Chargement des données...")
    train_dataset = create_dataset(DATASET_PATH, BATCH_SIZE, is_training=True)
    val_dataset = create_dataset(DATASET_PATH, BATCH_SIZE, is_training=False)
    
    print(f"✓ Dataset chargé")
    
    # Créer le modèle
    print(f"\n🏗️  Création du modèle...")
    network = EyeDiseaseNet(num_classes=NUM_CLASSES)
    
    # Loss pondérée
    loss_fn = WeightedLoss(class_weights)
    
    # Optimiseur
    optimizer = nn.Adam(
        network.trainable_params(),
        learning_rate=LEARNING_RATE,
        weight_decay=0.0001
    )
    
    model = Model(network, loss_fn=loss_fn, optimizer=optimizer)
    
    # Callbacks
    os.makedirs("./checkpoints_fixed", exist_ok=True)
    
    config_ck = CheckpointConfig(
        save_checkpoint_steps=train_dataset.get_dataset_size(), # Sauvegarder à la fin de chaque époque
        keep_checkpoint_max=20
    )
    ckpoint_cb = ModelCheckpoint(
        prefix="eye_disease_FIXED",
        directory="./checkpoints_fixed",
        config=config_ck
    )
    
    monitor_cb = DetailedMonitor(val_dataset, per_print_times=20)
    
    print(f"\n⚙️  Paramètres:")
    print(f"  - Epochs: {NUM_EPOCHS}")
    print(f"  - Batch size: {BATCH_SIZE}")
    print(f"  - Learning rate: {LEARNING_RATE}")
    print(f"  - Loss: Weighted Cross-Entropy")
    
    # Entraînement
    print(f"\n{'='*70}")
    print("DÉBUT DE L'ENTRAÎNEMENT")
    print(f"{'='*70}\n")
    
    model.train(
        NUM_EPOCHS,
        train_dataset,
        callbacks=[ckpoint_cb, monitor_cb],
        dataset_sink_mode=False
    )
    
    print(f"\n{'='*70}")
    print("ENTRAÎNEMENT TERMINÉ!")
    print(f"{'='*70}")
    print(f"\n✅ Meilleure accuracy: {monitor_cb.best_acc:.2f}%")
    print(f"✅ Modèles dans: ./checkpoints_fixed/")
    
    return model


# ============================================================================
# MAIN
# ============================================================================

if __name__ == "__main__":
    
    print("""
╔══════════════════════════════════════════════════════════════════╗
║            ENTRAÎNEMENT AVEC CORRECTIONS APPLIQUÉES              ║
╚══════════════════════════════════════════════════════════════════╝

✅ Loss pondérée (équilibre des classes)
✅ Learning rate réduit (0.0001)
✅ Augmentation agressive
✅ Batch size 32
✅ Monitoring détaillé

Ces corrections devraient résoudre le problème !
    """)
    
    input("\n👉 Appuyez sur Entrée pour commencer...")
    
    model = train_model()
    
    print("""
╔══════════════════════════════════════════════════════════════════╗
║                  ENTRAÎNEMENT TERMINÉ ✅                         ║
╚══════════════════════════════════════════════════════════════════╝

Testez maintenant avec:

    python test.py ./checkpoints_fixed/eye_disease_FIXED-50_XX.ckpt

Vous devriez voir des prédictions VARIÉES ! 🎯
    """)
